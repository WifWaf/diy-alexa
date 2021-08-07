#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "wav_reader.h"
#include "spiffs_assist.h"
#include "main_config.h"
#include "i2s_output.h"
#include "wav_player.h"
#include "ring_buffer.h"
#include "neural_network.h"
#include "application.h"
#include "i2s_mic_sampler.h"

#define TAG_MAIN "main.c"

wav_profile_t wav_ready_ping;
ring_buffer_t ring_buffer;
i2s_sampler_t i2s_sampler;
application_t application;

#define RDY_PING_PATH "/spiffs/ready_ping.wav"
#define RDY_CAT_PATH "/spiffs/white_cat.wav"

TaskHandle_t applicationTaskHandle;

void applicationTask(void *param);

/*
 IMPORTANT :
 Slower response times are due to C linkage with the neural network.

 Specifically measuring the elapse of m_interpreter->Invoke() shows
 this is the case.

 Untill the C api is released, litle can be done apart from optomising
 the mode.

 ** Memory leak also present - again, likely c linkage.
*/

void app_main(void)
{
  ESP_LOGI(TAG_MAIN, "Hello world!");

  // Get heap statts -------------------------------------------- //
  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
  size_t heap_size = info.total_free_bytes + info.total_allocated_bytes;

  ESP_LOGI(TAG_MAIN, "Total heap: %zu", heap_size);
  ESP_LOGI(TAG_MAIN, "Free heap: %zu", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  // ---- wake up sound ---------------------------------------- //
  wav_player_init(&i2s_output_spk);  
  wav_player_load(&wav_ready_ping, RDY_PING_PATH);
  wav_player_play(&wav_ready_ping);

  // stup mic input and associated sampler ------------------ //
  i2s_sampler_init(&i2s_sampler, &ring_buffer, I2S_NUM_0, &i2s_config_mems_dual_channel);
  i2s_mic_sampler_init(&i2s_sampler, &i2s_mic_pins);

  // app start ---------------------------------------------- // 
  application.ready_ping = &wav_ready_ping;  // test only

  application_init(&application, &i2s_sampler);
  xTaskCreate(applicationTask, "Application Task", 8192, &application, 1, &applicationTaskHandle);

  i2s_sampler_configure(&i2s_sampler, &i2s_mic_sampler_configure, &i2s_mic_sampler_process_data);

 // ESP_LOGI(TAG_MAIN, "Free heap: %zu", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  i2s_sampler_start(&i2s_sampler ,&applicationTaskHandle);
}

void applicationTask(void *param)
{
  application_t *app = (application_t *)(param);

  ESP_LOGI(TAG_MAIN, "Application starting");

  while (true)
  {
    // wait for some audio samples to arrive
    if(ulTaskNotifyTake(pdTRUE,  portMAX_DELAY))
    {
    //  ESP_LOGI(TAG_MAIN, "Notification given");
      application_run(app);
    }
  }
  vTaskDelete(NULL);   // should not reach here
}
