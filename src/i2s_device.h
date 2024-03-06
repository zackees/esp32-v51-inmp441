
#ifndef _I2S_DEVICE_H_
#define _I2S_DEVICE_H_

#include "defs.h"

#if AUDIO_BIT_RESOLUTION == 16 && AUDIO_CHANNELS == 1
typedef int16_t audio_sample_t;
#else
#error "Unsupported audio configuration"
#endif

typedef audio_sample_t audio_buffer_t[512];
typedef audio_sample_t dma_buffer_t[AUDIO_SAMPLES_PER_DMA_BUFFER];

void i2s_audio_init();
void i2s_audio_shutdown();
size_t i2s_read_samples(audio_sample_t* begin, audio_sample_t* end, uint32_t timeout);

inline size_t i2s_read_samples(audio_buffer_t buffer, uint32_t timeout) {
  // Legacy api
  audio_sample_t* begin = &buffer[0];
  audio_sample_t* end = begin + sizeof(audio_buffer_t) / sizeof(*begin);
  return i2s_read_samples(begin, end, timeout);
}

inline bool i2s_read_dma_sample(dma_buffer_t dma_buff, uint32_t timeout) {
  size_t samples_read = i2s_read_samples(dma_buff, dma_buff + AUDIO_SAMPLES_PER_DMA_BUFFER, timeout);
  return samples_read > 0;
}

void i2s_audio_enter_light_sleep();
void i2s_audio_exit_light_sleep();


#endif // _I2S_DEVICE_H_
