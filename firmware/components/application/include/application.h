#pragma once
#include "wav_player.h"
#include "i2s_sampler.h"
#include "detect_wake_word.h"


//class IndicatorLight;
//class Speaker;
//class IntentProcessor;

typedef struct
{
  bool _is_detect;
  detect_wake_word_state_t detect_wake_word_state;
  wav_profile_t *ready_ping;

  //  void(_detect_wake_word_state *)(detect_wake_word_state_t *info, i2s_sampler_t *sampler);
 //   State *m_detect_wake_word_state;
 //   State *m_recognise_command_state;
  //  _state *_current_state;

} application_t;

void application_init(application_t *info, i2s_sampler_t *sample_provider);
  //  ~Application();
void application_run(application_t *info);

