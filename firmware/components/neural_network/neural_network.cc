#include "neural_network.h"
#include "model.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <esp_log.h>
#include <esp_timer.h>

#define TAG_NN "Neural Network"

#define ARENA_SIZE 25000

namespace tflite
{
    template <unsigned int tOpCount>
    class MicroMutableOpResolver;
    class ErrorReporter;
    class Model;
    class MicroInterpreter;
} // namespace tflite

struct TfLiteTensor;

tflite::ErrorReporter* m_error_reporter;
const tflite::Model* m_model;
tflite::MicroInterpreter *m_interpreter;
TfLiteTensor *input;
TfLiteTensor *output;
tflite::MicroMutableOpResolver<10> *m_resolver;
alignas(16) uint8_t *m_tensor_arena;
//uint8_t *m_tensor_arena;
const int kArenaSize = 25000;                       // approximate working size of our model

float *neural_network_get_input_buffer()
{
   // neural_data_t *ptr = (neural_data_t*)info;
    return input->data.f;
}

float neural_network_predict()
{
    // neural_data_t *ptr = (neural_data_t*)info;
    //  int64_t start = esp_timer_get_time(); 
    m_interpreter->Invoke();
    // int64_t end_f = esp_timer_get_time(); 
    // ESP_LOGI("Time Test", "wake run elapse: %.f", (end_f - start)/1e3);
    return output->data.f[0];
}

void neural_network_uninit()
{
    delete m_interpreter;
    free(m_resolver); // ??? unsure how to delete <----- MEMORY LEAK 12 BYTES ********************************************
    free(m_tensor_arena);
    delete m_error_reporter;
}

void neural_network_init()
{
    // ESP_LOGI(TAG_NN, "Sizeof struct: %zu",  sizeof(neural_data_t));
    // neural_data_t *ptr = (neural_data_t *)malloc(sizeof(neural_data_t));
    m_error_reporter = new tflite::MicroErrorReporter();
    m_tensor_arena = (uint8_t *)malloc(kArenaSize);

    if (!m_tensor_arena)
    {
        TF_LITE_REPORT_ERROR(m_error_reporter, "Could not allocate arena");
        return;
    }
    TF_LITE_REPORT_ERROR(m_error_reporter, "Loading model");

    m_model = tflite::GetModel(converted_model_tflite);
    if (m_model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(m_error_reporter, "Model provided is schema version %d not equal to supported version %d.", 
        m_model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }
    // This pulls in the operators implementations we need
    m_resolver = new tflite::MicroMutableOpResolver<10>();
    m_resolver->AddConv2D();
    m_resolver->AddMaxPool2D();
    m_resolver->AddFullyConnected();
    m_resolver->AddMul();
    m_resolver->AddAdd();
    m_resolver->AddLogistic();
    m_resolver->AddReshape();
    m_resolver->AddQuantize();
    m_resolver->AddDequantize();

    // Build an interpreter to run the model with.
    m_interpreter = new tflite::MicroInterpreter(m_model, *m_resolver, 
                        m_tensor_arena, kArenaSize, m_error_reporter);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = m_interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(m_error_reporter, "AllocateTensors() failed");
        return;
    }

    size_t used_bytes = m_interpreter->arena_used_bytes();
    TF_LITE_REPORT_ERROR(m_error_reporter, "Used bytes %d\n", used_bytes);

    // Obtain pointers to the model's input and output tensors.
    input = m_interpreter->input(0);
    output = m_interpreter->output(0);
}