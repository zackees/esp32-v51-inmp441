
#ifndef _I2S_DEVICE_H_
#define _I2S_DEVICE_H_

#include "defs.h"

#if AUDIO_BIT_RESOLUTION == 16 && AUDIO_CHANNELS == 1
typedef int16_t audio_sample_t;
#else
#error "Unsupported audio configuration"
#endif

typedef audio_sample_t audio_buffer_t[AUDIO_SAMPLES_PER_DMA_BUFFER];

void i2s_audio_init();
void i2s_audio_shutdown();
size_t i2s_read_samples(audio_sample_t* buffer, size_t buffer_len);

inline size_t i2s_read_samples(audio_buffer_t buffer) {
  // Legacy api
  audio_sample_t* begin = &buffer[0];
  audio_sample_t* end = &buffer[AUDIO_SAMPLES_PER_DMA_BUFFER];
  return i2s_read_samples(begin, end - begin);
}


void i2s_audio_enter_light_sleep();
void i2s_audio_exit_light_sleep();
void i2s_isr_handler(void* arg);

// Counts the number of times the ISR has been called.
uint32_t i2s_get_dbg_counter();

#endif // _I2S_DEVICE_H_
