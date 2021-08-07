#include <driver/i2s.h>
#include "i2s_sampler.h"
#include "ring_buffer.h"
#include "i2s_mic_sampler.h"
#include <esp_log.h>

#define TAG_I2SS "I2S Sampler"

void xi2s_sampler_reader(void *param);

TaskHandle_t _reader_task_handle;                // processor task
QueueHandle_t _i2s_queue;                        // i2s port

void i2s_sampler_add_sample(i2s_sampler_t *info, int16_t sample)
{
    // store the sample
    ring_buffer_set_current_sample(info->ring_buffer, sample);
   // ESP_LOGI(TAG_I2SS, "Buffer Sample:  %d Sample %d", info->ring_buffer->_this_buffer->samples[info->ring_buffer->_pos], sample);
    if (ring_buffer_next_sample(info->ring_buffer))
    {
      // trigger the processor task as we've filled a buffer
      //   ESP_LOGI(TAG_I2SS, "NOTIFY");
      xTaskNotifyGive(*info->_processor_task_handle);
    }
}

void xi2s_sampler_reader(void *param)
{
    i2s_sampler_t *sampler = (i2s_sampler_t *)param;
    while (true)
    {
        i2s_event_t evt; // wait for some data to arrive on the queue
        if (xQueueReceive(_i2s_queue, &evt, portMAX_DELAY))
        {
            if (evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytesRead = 0;
                do
                {
                    uint8_t i2sData[1024];                                       // read data from the I2S peripheral
                    i2s_read(sampler->i2s_port, i2sData, 1024, &bytesRead, 10);  // read from i2s
                    sampler->i2s_process(i2sData, bytesRead);                    // process the raw data
                } while (bytesRead > 0);
            }
        }
    }
}

void i2s_sampler_init(i2s_sampler_t *info, ring_buffer_t *buff, i2s_port_t port, i2s_config_t *i2s_config)
{
    // port allocation
    info->i2s_port = port;
    info->_i2s_config = i2s_config;
    
    // allocate the audio buffers
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++)
    {
        audio_buffer_init(&info->_audio_buffers[i]);
    }

    info->ring_buffer = buff;

    ring_buffer_init(buff, info->_audio_buffers, AUDIO_BUFFER_COUNT);
    ring_buffer_set_index(buff, ring_buffer_get_index(buff));         // place the reaader at the same position as the writer - clients can move it around as required
}

void i2s_sampler_start(i2s_sampler_t *info, TaskHandle_t *processor_task_handle)
{
   ESP_LOGI(TAG_I2SS,"Starting i2s");

    info->_processor_task_handle = processor_task_handle;  
    //install and start i2s driver
    i2s_driver_install(info->i2s_port, info->_i2s_config, 4, &_i2s_queue);
    // set up the I2S
    info->i2s_configure();
    // start a task to read samples
    xTaskCreate(xi2s_sampler_reader, "i2s Reader Task", 4096, info, 1, &_reader_task_handle);
}

void i2s_sampler_set_ring_buffer_reader(i2s_sampler_t *info, ring_buffer_t *buff)
{
    ring_buffer_init(buff, info->_audio_buffers, AUDIO_BUFFER_COUNT);
    // place the reaader at the same position as the writer - clients can move it around as required
    ring_buffer_set_index(buff, ring_buffer_get_index(info->ring_buffer));
}