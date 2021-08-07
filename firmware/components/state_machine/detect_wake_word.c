#include "detect_wake_word.h"
#include <esp_log.h>

#define TAG_DWW "Detect Wake Word"

#define TRIGG_THRESHOLD 0.85

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

void detect_wake_word_state_init(detect_wake_word_state_t *info, i2s_sampler_t *sampler)
{
    // save the sample provider for use later
    info->_sample_provider = sampler;
    // some stats on performance
    info->_average_detect_time = 0;
    info->_number_of_runs = 0;
}

void detect_wake_word_state_enter_state(detect_wake_word_state_t *info)
{
    // Create our neural network
    neural_network_init();
    ESP_LOGI(TAG_DWW, "Created Neral Net");
    // create our audio processor
    info->_audio_pro = (audio_processor_t *)malloc(sizeof(audio_processor_t));
    audio_processor_init(info->_audio_pro, AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);  
    ESP_LOGI(TAG_DWW, "Created audio processor");
    info->_number_of_detections = 0;
}

bool detect_wake_word_state_run(detect_wake_word_state_t *info)
{
    // time how long this takes for stats
    int64_t start = esp_timer_get_time();   
    // get access to the samples that have been read in
    info->_ring_buffer = malloc(sizeof(ring_buffer_t ));
    i2s_sampler_set_ring_buffer_reader(info->_sample_provider, info->_ring_buffer);
    // rewind by 1 second 
    ring_buffer_rewind(info->_ring_buffer, 16000);
    // get hold of the input buffer for the neural network so we can feed it data
    float *input_buffer = neural_network_get_input_buffer();
    audio_processor_get_spectrogram(info->_audio_pro, info->_ring_buffer,  input_buffer);
    // finished with the sample reader
    free(info->_ring_buffer);
    // get the prediction for the spectrogram
    float output = neural_network_predict();
    int64_t  end = esp_timer_get_time();
    // compute the stats
    info->_average_detect_time = ((end - start) * 0.1) + (info->_average_detect_time * 0.9); 
    info->_number_of_runs++;
    // log out some timing info
    if (info->_number_of_runs == 100)
    {
        info->_number_of_runs = 0;
        ESP_LOGI(TAG_DWW, "Average detection time %.1fms", info->_average_detect_time/1e3); // us to ms
    }
    // use quite a high threshold to prevent false positives
    if (output > TRIGG_THRESHOLD)
    {
        info->_number_of_detections++;
        if (info->_number_of_detections > 1)
        {
            info->_number_of_detections = 0;
            // detected the wake word in several runs, move to the next state
            ESP_LOGI(TAG_DWW, "P(%.2f): Here I am, brain the size of a planet...", output);
            return true;
        }
    }
    // nothing detected stay in the current state

    return false;
}

void detect_wake_word_state_exit_state(detect_wake_word_state_t *info)
{
    // Delete our neural network
    neural_network_uninit();
    audio_processor_uninit(info->_audio_pro);
    free(info->_audio_pro);
    uint32_t free_ram = esp_get_free_heap_size();
    ESP_LOGI(TAG_DWW, "Free ram after DetectWakeWord cleanup %d\n", free_ram);
}