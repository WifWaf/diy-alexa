  
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void neural_network_init();
float *neural_network_get_input_buffer();
float neural_network_predict();
void neural_network_uninit();

#ifdef __cplusplus
}
#endif