#include "application.h"
//#include "state_machine/RecogniseCommandState.h"
//#include "IndicatorLight.h"
//#include "Speaker.h"
//#include "IntentProcessor.h"

void application_init(application_t *info, i2s_sampler_t *sample_provider)
{
  detect_wake_word_state_init(&info->detect_wake_word_state, sample_provider);    
  // detect wake word state - waits for the wake word to be detected
  // m_detect_wake_word_state = new DetectWakeWordState(sample_provider);
  // command recongiser - streams audio to the server for recognition
  //  m_recognise_command_state = new RecogniseCommandState(sample_provider, indicator_light, speaker, intent_processor);
  // start off in the detecting wakeword state
  info->_is_detect = true;
  detect_wake_word_state_enter_state(&info->detect_wake_word_state);  
}


// process the next batch of samples
void application_run(application_t *info)
{
  
  // bool state_done = (info->_is_detect) ? detect_wake_word_state_run(info->detect_wake_word_state) : 0;                                                            
  bool state_done = detect_wake_word_state_run(&info->detect_wake_word_state);

  if (state_done)
  {
    wav_player_play(info->ready_ping);
    //  m_current_state->exitState();
    detect_wake_word_state_exit_state(&info->detect_wake_word_state);  

      // switch to the next state - very simple state machine so we just go to the other state...
      /*
      nfo->_is_detect = !nfo->_is_detect;
      if (m_current_state == m_detect_wake_word_state)
      {
          m_current_state = m_recognise_command_state;
      }
      else
      {
          m_current_state = m_detect_wake_word_state;
      }
      */
    detect_wake_word_state_enter_state(&info->detect_wake_word_state);
    //  m_current_state->enterState();
  }
  vTaskDelay(10);
} 
