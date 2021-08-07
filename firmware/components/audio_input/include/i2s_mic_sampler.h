#pragma once

#include "i2s_sampler.h"
#include "soc/i2s_reg.h"

#ifndef SPH0645
#define SPH0645 0
#endif

void i2s_mic_sampler_init(i2s_sampler_t *info, i2s_pin_config_t *pins_info);
void i2s_mic_sampler_configure();
void i2s_mic_sampler_process_data(uint8_t *i2sData, size_t bytesRead);