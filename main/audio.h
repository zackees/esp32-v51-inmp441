#ifndef _AUDIOSENSOR_H_
#define _AUDIOSENSOR_H_

#include <stdint.h>
#include "defs.h"
#include <cstring>
#include "i2s_device.h"



struct audio_state_t {
  float db = 0.f;
  uint32_t updated = 0;
  audio_buffer_t samples = {0};
  audio_state_t(float _db, uint32_t _updated, const audio_buffer_t& samples) : db(_db), updated(_updated) {
    memcpy(this->samples, samples, sizeof(this->samples));
  }
};

void audio_init(bool wait_for_powerup = true);
void audio_shutdown();
// The last loudness_dB set is returned.
audio_state_t audio_update();

void audio_buffer_clear();
void audio_loudness_test();

float audio_loudness_dB();

// Power functions

void audio_enter_light_sleep();
void audio_exit_light_sleep();

#endif // _AUDIOSENSOR_H_
