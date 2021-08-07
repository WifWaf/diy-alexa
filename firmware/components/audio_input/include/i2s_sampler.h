#pragma once

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <driver/i2s.h>
#include "i2s_sampler.h"
#include "ring_buffer.h"

#define AUDIO_BUFFER_COUNT 11

typedef struct 
{
    samples_buff_t _audio_buffers[AUDIO_BUFFER_COUNT];          // audio buffers
    ring_buffer_t *ring_buffer;    
    
    uint16_t _this_audio_buffer;                                 // I2S reader task
    void (*i2s_process)(uint8_t *i2sData, size_t bytesRead);     // pointer to configure task
    void (*i2s_configure)();                                     // pointer to configure task

    TaskHandle_t *_processor_task_handle;                        // i2s reader queue
    i2s_port_t i2s_port;        
    i2s_config_t *_i2s_config;
    i2s_pin_config_t *_pin_config;
} i2s_sampler_t;

void i2s_sampler_init(i2s_sampler_t *info, ring_buffer_t *buff, i2s_port_t port, i2s_config_t *i2s_config);
void i2s_sampler_add_sample(i2s_sampler_t *info, int16_t sample);
void i2s_sampler_start(i2s_sampler_t *info, TaskHandle_t *processor_task_handle);

inline void i2s_sampler_configure(i2s_sampler_t *info, void (*i2s_configure)(void), void (*i2s_process)){
    info->i2s_process = i2s_process;
    info->i2s_configure = i2s_configure;
}

inline void i2s_sampler_process_i2s_data(i2s_sampler_t *info, uint8_t *i2sData, size_t bytesRead) {
    info->i2s_process(i2sData, bytesRead);
}

inline i2s_port_t i2s_sampler_get_i2s_port(i2s_sampler_t *info) {
    return info->i2s_port;
}

inline uint32_t i2s_sampler_get_write_position(i2s_sampler_t *info) {
    return  ring_buffer_get_index(info->ring_buffer);
}

inline uint16_t i2s_sampler_get_ring_buffer_sze() {
    return AUDIO_BUFFER_COUNT * SAMPLE_BUFFER_SIZE;
}

void i2s_sampler_set_ring_buffer_reader(i2s_sampler_t *info, ring_buffer_t *buff);