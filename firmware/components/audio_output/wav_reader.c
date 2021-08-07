
#include <esp_log.h>
#include "wav_reader.h"

#define TAG_WAV "Wav Reader"

typedef struct {
    // RIFF Header
    uint8_t riff_header[4];     // Contains "RIFF"
    uint32_t wav_size;          // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    uint8_t wave_header[4];     // Contains "WAVE"

    // Format Header
    uint8_t fmt_header[4];      // Contains "fmt " (includes trailing space)
    uint32_t fmt_chunk_size;    // Should be 16 for PCM
    uint16_t audio_format;      // Should be 1 for PCM. 3 for IEEE Float
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;         // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    uint16_t sample_alignment;  // num_channels * Bytes Per Sample
    uint16_t bit_depth;         // Number of bits per sample

    // Data
    uint8_t data_header[4];     // Contains "data"
    uint32_t data_bytes;        // Number of bytes in data. Number of samples * num_channels * sample byte size
} wav_header_t;

esp_vfs_spiffs_conf_t _conf = 
{
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 9,
    .format_if_mount_failed = false
};

esp_vfs_spiffs_conf_t *conf = NULL;

void _wav_priv_reader_reset(wav_profile_t *wav_prof);

void wav_reader_set_cfg(esp_vfs_spiffs_conf_t *config)
{  
    conf = config;                // Load spiffs config into assistant  
}

void wav_reader_init(wav_profile_t *wav_prof, bool repeat)
{
    if(!conf)
    {
        ESP_LOGI(TAG_WAV, "applying default esp_vfs_spiffs_conf_t config");
        wav_reader_set_cfg(&_conf);
    }
    
    if(!wav_prof->full_path)       // is file name present?
    {
        ESP_LOGE(TAG_WAV, "wav path not provided");
        return;
    }
    
    wav_header_t wav_header;

    spiffs_assist_init(conf);           // start spiffs_assist. Must be loaded first as get path depends esp_vfs_spiffs_conf_t data.
    spiffs_assist_open(wav_prof->full_path);
    wav_prof->_available = spiffs_assist_read((char *)&wav_header, sizeof(wav_header_t));       // Read file into wav header
   
    if (wav_header.bit_depth != 16)     // sanity check the bit depth
        ESP_LOGE(TAG_WAV, "ERROR: bit depth %d is not supported please use 16 bit signed integer", wav_header.bit_depth);

    if (wav_header.sample_rate != 16000)
        ESP_LOGE(TAG_WAV, "ERROR: bit depth %d is not supported please us 16KHz", wav_header.sample_rate);
    
    ESP_LOGI(TAG_WAV, "fmt_chunk_size: %d audio_format: %d num_channels: %d sample_rate: %d sample_alignment: %d bit_depth: %d data_bytes: %d",
                  wav_header.fmt_chunk_size, wav_header.audio_format, wav_header.num_channels, 
                  wav_header.sample_rate, wav_header.sample_alignment, wav_header.bit_depth, 
                  wav_header.data_bytes);

    spiffs_assist_get_pos(&wav_prof->_pos_header);   // store end of header position
    spiffs_assist_get_pos(&wav_prof->_pos_last);     // align with current position

    spiffs_assist_close();

    wav_prof->_channels = wav_header.num_channels; 
    wav_prof->_repeat = repeat; 
}

void wav_reader_reset(wav_profile_t *wav_prof)
{
    spiffs_assist_open(wav_prof->full_path);                                  // open SPIFFS
    wav_prof->_available = spiffs_assist_set_pos(&wav_prof->_pos_header);     // goto header end  
    spiffs_assist_get_pos(&wav_prof->_pos_last);                              // align with current position
    spiffs_assist_close();
}

void _wav_priv_reader_reset(wav_profile_t *wav_prof)
{
    wav_prof->_available = spiffs_assist_set_pos(&wav_prof->_pos_header);      // goto header end  
    spiffs_assist_get_pos(&wav_prof->_pos_last);                               // align with current position
}

uint16_t wav_reader_get_frames(wav_profile_t *wav_prof, Frame_t *frames, uint16_t number_frames)
{
    spiffs_assist_open(wav_prof->full_path);                                   // open SPIFFS
    wav_prof->_available = spiffs_assist_set_pos(&wav_prof->_pos_last);        // restore last known position
    uint16_t i;
    for(i = 0; i < number_frames; i++)               // fill the buffer with data from the file wrapping around if necessary
    {
        if(!wav_prof->_available)                    // at end of file?
        {
            if(wav_prof->_repeat)
                _wav_priv_reader_reset(wav_prof);    // move back to the start of the file and carry on
            else
            {       
                number_frames = i;                    // we've reached the end of the file, return the number of frames we were able to fill 
                break;
            }                       
        }

        wav_prof->_available = spiffs_assist_read((char *)&frames[i].left, sizeof(int16_t));        // read in the next sample to the left channel
        
        if (wav_prof->_channels)     
            frames[i].right = frames[i].left;        // if we only have one channel duplicate the sample for the right channel
        else       
            wav_prof->_available = spiffs_assist_read((char *)&frames[i].right, sizeof(int16_t));   // otherwise read in the right channel sample
    }

    spiffs_assist_get_pos(&wav_prof->_pos_last);          // update file position
    spiffs_assist_close();                                // close SPIFFS

    return number_frames;    
}

bool wav_reader_available(wav_profile_t *wav_prof)
{
    return wav_prof->_available || wav_prof->_repeat; 
}