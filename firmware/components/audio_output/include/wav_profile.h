#ifndef __WAV_PROFILE_H__
#define __WAV_PROFILE_H__

#include "stdint.h"
#include "spiffs_assist.h"
#include <string.h>

typedef struct {
    fpos_t _pos_header, _pos_last;   // FILE position of header and last read/write 
    uint8_t _channels;               // channels, mono or stereo
    bool _repeat;                    // multiple play 
    bool _available;                 // FILE end detected = false
    
    char *full_path;                 // full wav path to file. I.e. "/spiffs/sound.wav"
} wav_profile_t;

inline void wav_profile_create(wav_profile_t *profile, char *wav_full_path) { 
    memset(profile, 0, sizeof(wav_profile_t));
    profile->full_path = wav_full_path;
}

inline void wav_proifle_remove(wav_profile_t *profile)
{
    memset(profile, 0, sizeof(wav_profile_t));
    profile->full_path = NULL;
}

#endif // __WAV_PROFILE_H__