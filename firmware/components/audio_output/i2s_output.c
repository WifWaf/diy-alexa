#include <math.h>
#include "i2s_output.h"

#define FRAME_CNT 128                                   // number of frames to try and send at once (a frame is a left and right sample)
#define TAG_I2S "I2S Output"

TaskHandle_t i2s_writer_task_h = NULL;                  // I2S write task
QueueHandle_t i2s_q = NULL, i2s_send_q = NULL;          // i2s writer queue

void xi2s_output_writer(void *param);

void i2s_output_sample_generator_push(wav_profile_t *profile)
{
    if(!profile)
    {
        ESP_LOGE(TAG_I2S, "wav_reader_t is NULL"); 
        return; 
    }

    if(!xQueueSend(i2s_send_q, profile,  pdMS_TO_TICKS(100)))  
        ESP_LOGE(TAG_I2S, "Time out sending to i2s_send_q");  
}
void xi2s_output_writer(void *param)
{
    i2s_event_t evt;
    i2s_output_t *output_cfg = (i2s_output_t *)param;
    wav_profile_t wav_profile;
    Frame_t frame_data[FRAME_CNT];

    for(;;)
    {     
        while(xQueueReceive(i2s_q, &evt, portMAX_DELAY))      // wait for some data to be requested
        {
            if(evt.type == I2S_EVENT_TX_DONE)
            {
                if(xQueueReceive(i2s_send_q, &wav_profile, 0)) // wait for some data to be requested
                {
                    ESP_LOGI(TAG_I2S, "Generating sample");
                    do
                    {
                        uint16_t frame_count = wav_reader_get_frames(&wav_profile, frame_data, FRAME_CNT);       // get some frames from the wave file - a frame consists of a 16 bit left and right sample
                        uint16_t bytes_to_send = frame_count * sizeof(uint16_t) * 2;                             // 2 bytes per frame * 2 channels

                        if(bytes_to_send > 0)                  // do we have something to write?
                        {
                            size_t bytes_sent = 0;
                            i2s_write(output_cfg->i2s_port, (uint8_t *)frame_data, bytes_to_send, &bytes_sent, pdMS_TO_TICKS(100));  // write data to the i2s peripheral, 100 ms time out                             
                            bytes_to_send -= bytes_sent;       // how much was sent?

                            if(bytes_to_send)
                                ESP_LOGW(TAG_I2S,"i2s_write timed out writting frame data. %d bytes were lost", bytes_to_send);
                        }
                    } while(wav_profile._available);
                    ESP_LOGI(TAG_I2S, "Fininished generating sample");
                }
            } 
        }
    }
}

/*
void xi2s_output_writer(void *param)
{
    i2s_event_t evt;
    i2s_output_t *output_cfg = (i2s_output_t *)param;
    wav_profile_t wav_profile;
    Frame_t frame_data[FRAME_CNT];

    for(;;)
    {     
        while(xQueueReceive(i2s_q, &evt, portMAX_DELAY))      // wait for some data to be requested
        {
            if(evt.type == I2S_EVENT_TX_DONE)
            {
                if(xQueueReceive(i2s_send_q, &wav_profile, 0)) // wait for some data to be requested
                {
                    ESP_LOGI(TAG_I2S, "Generating sample");
                    do
                    {
                        uint16_t frame_count = wav_reader_get_frames(&wav_profile, frame_data, FRAME_CNT);       // get some frames from the wave file - a frame consists of a 16 bit left and right sample
                        uint16_t bytes_to_send = frame_count * sizeof(uint16_t) * 2;                             // 2 bytes per frame * 2 channels

                        if(bytes_to_send > 0)                  // do we have something to write?
                        {
                            size_t bytes_sent = 0;
                            i2s_write(output_cfg->i2s_port, (uint8_t *)frame_data, bytes_to_send, &bytes_sent, pdMS_TO_TICKS(100));  // write data to the i2s peripheral, 100 ms time out                             
                            bytes_to_send -= bytes_sent;       // how much was sent?

                            if(bytes_to_send)
                                ESP_LOGW(TAG_I2S,"i2s_write timed out writting frame data. %d bytes were lost", bytes_to_send);
                        }
                    } while(wav_profile._available);
                    ESP_LOGI(TAG_I2S, "Fininished generating sample");
                }
            } 
        }
    }
}
*/

void i2s_output_start(i2s_output_t *info)
{
    i2s_driver_install(info->i2s_port, info->i2s_config, 4, &i2s_q);  //install and start i2s driver
    i2s_set_pin(info->i2s_port, info->pin_config);                    // set up the i2s pins
    i2s_zero_dma_buffer(info->i2s_port);                              // clear the DMA buffers

    i2s_send_q = xQueueCreate(5, sizeof(wav_profile_t));

    xTaskCreate(xi2s_output_writer, "i2s output writer", 4096, info, 1, &i2s_writer_task_h);   // start a task to write samples to the i2s peripheral
}
void i2s_output_stop(i2s_output_t *info)
{
    if(!i2s_writer_task_h)
    {
        ESP_LOGE(TAG_I2S, "No task to stop");
        return;
    }

    vTaskDelete(i2s_writer_task_h);
    vQueueDelete(i2s_send_q);
    i2s_driver_uninstall(info->i2s_port);
}