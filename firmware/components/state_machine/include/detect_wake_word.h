#pragma once

#include <stdbool.h>
#include "i2s_sampler.h"
#include "audio_processor.h"
#include "ring_buffer.h"
#include "neural_network.h"

typedef struct
{
    i2s_sampler_t *_sample_provider;
    ring_buffer_t *_ring_buffer;    
    audio_processor_t *_audio_pro;

    float _average_detect_time;
    int _number_of_detections;
    int _number_of_runs;
} detect_wake_word_state_t;

void detect_wake_word_state_init(detect_wake_word_state_t *info, i2s_sampler_t *sampler);
void detect_wake_word_state_enter_state(detect_wake_word_state_t *info);
bool detect_wake_word_state_run(detect_wake_word_state_t *info);
void detect_wake_word_state_exit_state(detect_wake_word_state_t *info);
