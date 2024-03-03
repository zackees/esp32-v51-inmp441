
#ifndef _I2S_DEVICE_H_
#define _I2S_DEVICE_H_

#include "defs.h"

#if AUDIO_BIT_RESOLUTION == 16 && AUDIO_CHANNELS == 1
typedef int16_t audio_sample_t;
#else
#error "Unsupported audio configuration"
#endif

typedef audio_sample_t audio_buffer_t[512];

void i2s_audio_init();
void i2s_audio_shutdown();
size_t i2s_read_samples(audio_sample_t* begin, audio_sample_t* end);

inline size_t i2s_read_samples(audio_buffer_t buffer) {
  // Legacy api
  audio_sample_t* begin = &buffer[0];
  audio_sample_t* end = &buffer[AUDIO_SAMPLES_PER_DMA_BUFFER];
  return i2s_read_samples(begin, end);
}


void i2s_audio_enter_light_sleep();
void i2s_audio_exit_light_sleep();


#endif // _I2S_DEVICE_H_
