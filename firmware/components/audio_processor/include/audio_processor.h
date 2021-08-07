#pragma once

#include <stdlib.h>
#include <stdint.h>
// #define FIXED_POINT (16)
#include "kiss_fftr.h"
#include "hamming_window.h"
#include "ring_buffer.h"

typedef struct
{
    int _audio_length;
    int _window_size;
    int _step_size;
    int _pooling_size;
    size_t _fft_size;
    float *_fft_input;
    int _energy_size;
    int _pooled_energy_size;
    float *_energy;
    kiss_fft_cpx *_fft_output;
    kiss_fftr_cfg _cfg;

    hamming_window_t _hamming_window;
}audio_processor_t;


void audio_processor_get_spectrogram_segment(audio_processor_t *info, float *output_spectrogram_row);
void audio_processor_init(audio_processor_t *info, int audio_length, int window_size, int step_size, int pooling_size);
void audio_processor_uninit(audio_processor_t *info);
void audio_processor_get_spectrogram(audio_processor_t *info, ring_buffer_t *ring_buff, float *output_spectrogram);
