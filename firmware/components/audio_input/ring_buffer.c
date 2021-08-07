#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t *info, samples_buff_t *samples_buff, uint16_t buffer_count)
{   
    info->_pos = 0;
    info->_idx = 0;
    info->_buffer_cnt = buffer_count;
    info->buffers = samples_buff;
    info->_size = info->_buffer_cnt * SAMPLE_BUFFER_SIZE;
    info->_this_buffer = &samples_buff[0];
}