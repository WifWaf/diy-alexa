#pragma once

#include <inttypes.h>

typedef struct 
{
    float *_coefficients;
    int _window_size;
} hamming_window_t;

void hamming_window_init(hamming_window_t *info, int window_size);
void hamming_window_uninit(hamming_window_t *info);
void hamming_window_apply_window(hamming_window_t *info, float *input);