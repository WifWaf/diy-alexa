#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "hamming_window.h"
#include <esp_log.h>

#define TAG_HAM "Hamming Window"

void hamming_window_init(hamming_window_t *info, int window_size)
{
    ESP_LOGI(TAG_HAM, "Starting hamming window..");
    info->_window_size = window_size;
    info->_coefficients = (float *)malloc(sizeof(float) * info->_window_size);
    // create the constants for a hamming window
    const float arg = M_PI * 2.0 / window_size;
    for (int i = 0; i < window_size; i++)
    {
        float float_value = 0.5 - (0.5 * cos(arg * (i + 0.5)));
        // Scale it to fixed point and round it.
        info->_coefficients[i] = float_value;
    }
}

void hamming_window_uninit(hamming_window_t *info)
{
    free(info->_coefficients);
}

void hamming_window_apply_window(hamming_window_t *info, float *input)
{
  //  ESP_LOGI(TAG_HAM, "Applying Window: %d", info->_window_size);
    for (uint16_t i = 0; i < info->_window_size; i++)
    {
        input[i] = input[i] * info->_coefficients[i];
    }
}
