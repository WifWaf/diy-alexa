#include <stdlib.h>
#include "audio_processor.h"
#include "hamming_window.h"
#include "ring_buffer.h"
#include "esp_timer.h"
#include <esp_log.h>

#define EPSILON 1e-6
#define TAG_APRO "Audio Processor"

void audio_processor_init(audio_processor_t *info, int audio_length, int window_size, int step_size, int pooling_size)
{
    info->_audio_length = audio_length;
    info->_window_size = window_size;
    info->_step_size = step_size;
    info->_pooling_size = pooling_size;
    info->_fft_size = 1;

    while (info->_fft_size < window_size)
    {
        info->_fft_size <<= 1;
    }
    info->_fft_input = (float *)malloc(sizeof(float) * info->_fft_size);
    info->_energy_size = info->_fft_size / 2 + 1;
    info->_fft_output = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx) * info->_energy_size);
    info->_energy = (float *)malloc(sizeof(float) * info->_energy_size);
    // work out the pooled energy size
    info->_pooled_energy_size = ceilf((float)info->_energy_size / (float)pooling_size);

    ESP_LOGI(TAG_APRO, "m_pooled_energy_size=%d", info->_pooled_energy_size);
    // initialise kiss fftr
    info->_cfg = kiss_fftr_alloc(info->_fft_size, false, 0, 0);
    // initialise the hamming window
    hamming_window_init(&info->_hamming_window, window_size);
}

void audio_processor_uninit(audio_processor_t *info)
{
    free(info->_cfg);
    free(info->_fft_input);
    free(info->_fft_output);
    free(info->_energy);
    hamming_window_uninit(&info->_hamming_window);
}

// takes a normalised array of input samples of window_size length
void audio_processor_get_spectrogram_segment(audio_processor_t *info, float *output)
{
    // apply the hamming window to the samples
    hamming_window_apply_window(&info->_hamming_window, info->_fft_input);
    // do the fft
    kiss_fftr(info->_cfg, info->_fft_input, (kiss_fft_cpx *)(info->_fft_output));
    // pull out the magnitude squared values
    for (int i = 0; i < info->_energy_size; i++)
    {
        const float real = info->_fft_output[i].r;
        const float imag = info->_fft_output[i].i;
        const float mag_squared = (real * real) + (imag * imag);
        info->_energy[i] = mag_squared;
    }
    // reduce the size of the output by pooling with average and same padding
    float *output_src = info->_energy;
    float *output_dst = output;
    for (int i = 0; i < info->_energy_size; i += info->_pooling_size)
    {
        float average = 0;
        for (int j = 0; j < info->_pooling_size; j++)
        {
            if (i + j < info->_energy_size)
            {
                average += *output_src;
                output_src++;
            }
        }
        *output_dst = average / info->_pooling_size;
        output_dst++;
    }
    // now take the log to give us reasonable values to feed into the network
    for (int i = 0; i < info->_pooled_energy_size; i++)
    {
        output[i] = log10f(output[i] + EPSILON);
    }
}

void audio_processor_get_spectrogram(audio_processor_t *info, ring_buffer_t *ring_buff, float *output_spectrogram)
{
  //  ESP_LOGI("TAG_APRO", "Generating Spectrogram");
 //   int64_t start = esp_timer_get_time(); 
     
    int startIndex = ring_buffer_get_index(ring_buff);

    // get the mean value of the samples
    float mean = 0;
    for (int i = 0; i < info->_audio_length; i++)
    {
        mean += ring_buffer_get_current_sample(ring_buff);
        ring_buffer_next_sample(ring_buff);
    }
    mean /= info->_audio_length;
    // get the absolute max value of the samples taking into account the mean value
    ring_buffer_set_index(ring_buff, startIndex);
    float max = 0, csample = 0;
    for (int i = 0; i < info->_audio_length; i++)
    {
        csample = fabsf((float)ring_buffer_get_current_sample(ring_buff) - mean);
        max = (max < csample) ?  csample : max;
        ring_buffer_next_sample(ring_buff);
    }
    // extract windows of samples moving forward by step size each time and compute the spectrum of the window
    for (int window_start = startIndex; window_start < startIndex + 16000 - info->_window_size; window_start += info->_step_size)
    {
        // move the reader to the start of the window
        ring_buffer_set_index(ring_buff, window_start);
        // read samples from the reader into the fft input normalising them by subtracting the mean and dividing by the absolute max
        for (int i = 0; i < info->_window_size; i++)
        {
            info->_fft_input[i] = ((float)ring_buffer_get_current_sample(ring_buff) - mean) / max;
             ring_buffer_next_sample(ring_buff);
        }
        // zero out whatever else remains in the top part of the input.
       // ESP_LOGI("TAG_APRO", "_window_size: %d  info->_fft_size: %d", info->_window_size, info->_fft_size);
        for (int i = info->_window_size; i < info->_fft_size; i++)
        {
            info->_fft_input[i] = 0;
        }
        // compute the spectrum for the window of samples and write it to the output
        audio_processor_get_spectrogram_segment(info, output_spectrogram);
        // move to the next row of the output spectrogram
        output_spectrogram += info->_pooled_energy_size;
    }
 //   int64_t end = esp_timer_get_time(); 
 //   ESP_LOGI("TAG_APRO", "Spectrogram time: %.f", (end - start)/1e3);
}