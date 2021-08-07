#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define SAMPLE_BUFFER_SIZE 1600

typedef struct
{
    int16_t samples[SAMPLE_BUFFER_SIZE];
} samples_buff_t;

typedef struct 
{
    samples_buff_t *buffers;
    samples_buff_t *_this_buffer;
    uint32_t _buffer_cnt;
    uint32_t _pos;
    uint32_t _idx;
    uint32_t _size;
} ring_buffer_t;

inline void audio_buffer_init(samples_buff_t *samples) {  
    memset(samples, 0, sizeof(samples_buff_t));
}

void ring_buffer_init(ring_buffer_t *info, samples_buff_t *samples_buff, uint16_t buffer_count);

inline bool ring_buffer_next_sample(ring_buffer_t *info)
{
    info->_pos++;
    if (info->_pos == SAMPLE_BUFFER_SIZE)
    {
        info->_pos = 0;
        info->_idx++;
        if (info->_idx == info->_buffer_cnt)
        {
            info->_idx = 0;
        }
        info->_this_buffer = &info->buffers[info->_idx];
        return true;
    }
    return false;
}

inline void ring_buffer_set_index(ring_buffer_t *info, uint32_t index)
{
    index = (index + info->_size) % info->_size;                        // handle negative indexes
    info->_idx = (index / SAMPLE_BUFFER_SIZE) % info->_buffer_cnt;      // work out which buffer
    info->_pos = index % SAMPLE_BUFFER_SIZE;                            // and where we are in the buffer
    info->_this_buffer = &info->buffers[info->_idx];
}

inline uint32_t ring_buffer_get_index(ring_buffer_t *info) { 
    return info->_idx * SAMPLE_BUFFER_SIZE + info->_pos; 
}
inline int16_t ring_buffer_get_current_sample(ring_buffer_t *info) { 
    return info->_this_buffer->samples[info->_pos]; 
}
inline void ring_buffer_set_current_sample(ring_buffer_t *info, int16_t sample) {
    info->_this_buffer->samples[info->_pos] = sample; 
}
inline void ring_buffer_rewind(ring_buffer_t *info, int16_t samples) { 
    ring_buffer_set_index(info, ring_buffer_get_index(info) - samples); 
}

#ifdef __cplusplus
}
#endif