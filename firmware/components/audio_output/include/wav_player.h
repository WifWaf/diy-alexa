#ifndef __WAV_PLAYER_H_
#define __WAV_PLAYER_H_

#include "wav_profile.h"
#include "i2s_output.h"

inline void wav_player_init(i2s_output_t *i2s_config)
{
    i2s_output_start(i2s_config);                 // start i2s output 
}

inline void wav_player_load(wav_profile_t *wav_profile, char *wav_path)
{
    wav_profile_create(wav_profile, wav_path);    // assign memory to wav_profile_t pointer
    wav_reader_init(wav_profile, false);          // load essentials into wav_profile_t
}   

inline void wav_player_play(wav_profile_t *wav_profile)
{
   wav_reader_reset(wav_profile);                  // make sure file is at the start of the wav data
   i2s_output_sample_generator_push(wav_profile);  // send to i2s_output queue for writing
}

inline void wav_player_uninit(i2s_output_t *i2s_config)
{
    i2s_output_stop(i2s_config);                  // uninitialise i2s output
}

inline void wav_player_unload(wav_profile_t *wav_profile)
{
    wav_proifle_remove(wav_profile);              // uninitialise wav profiles
}   

#endif // __WAV_PLAYER_H_