#include "i2s_sampler.h"
#include "i2s_mic_sampler.h"
#include "soc/i2s_reg.h"
#include "esp_log.h"

#define TAG_MICS "I2S Microphone Sampler"

i2s_sampler_t *s;

void i2s_mic_sampler_init(i2s_sampler_t *info, i2s_pin_config_t *pins_info)
{
    s = info;
    s->_pin_config = pins_info;
}

void i2s_mic_sampler_configure()
{
    if (SPH0645)
    {
        // FIXES for SPH0645
        ESP_LOGI(TAG_MICS, "Applying SPH0645 Fix");
        REG_SET_BIT(I2S_TIMING_REG(s->i2s_port), BIT(9));
        REG_SET_BIT(I2S_CONF_REG(s->i2s_port), I2S_RX_MSB_SHIFT);
    }

    i2s_set_pin(s->i2s_port, s->_pin_config);
}

void i2s_mic_sampler_process_data(uint8_t *i2sData, size_t bytesRead)
{
    int32_t *samples = (int32_t *)i2sData;                             // process the raw data
    
    for (uint16_t i = 0; i < bytesRead / 4; i++)
    {
      //  ESP_LOGI(TAG_MICS, "%d", samples[i] >> 11);
        i2s_sampler_add_sample(s, samples[i] >> 11);        
    }
}