#ifndef __I2S_OUTPUT_H__
#define __I2S_OUTPUT_H__

#include "driver/i2s.h"
#include "wav_reader.h"
#include "wav_profile.h"

typedef struct 
{
    i2s_port_t i2s_port;
    i2s_config_t *i2s_config;
    i2s_pin_config_t *pin_config;
} i2s_output_t;

void i2s_output_start(i2s_output_t *info);
void i2s_output_stop(i2s_output_t *info);
void i2s_output_sample_generator_push(wav_profile_t *wav);   // set generator to a wav file (only written to i2s if file is not at the end)

#endif // __I2S_OUTPUT_H__