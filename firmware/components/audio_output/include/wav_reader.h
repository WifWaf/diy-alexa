#ifndef __wav_file_reader_h__
#define __wav_file_reader_h__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <esp_spiffs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/types.h>

#include "spiffs_assist.h"
#include "wav_profile.h"

typedef struct
{
    int16_t left;
    int16_t right;
} Frame_t;

void wav_reader_init(wav_profile_t *wav_profile, bool repeat);
void wav_reader_set_cfg(esp_vfs_spiffs_conf_t *config);

uint16_t wav_reader_get_frames(wav_profile_t *wav_profile, Frame_t *frames, uint16_t number_frames);
bool wav_reader_available(wav_profile_t *wav_profile);
void wav_reader_reset(wav_profile_t *wav_profile);

#endif