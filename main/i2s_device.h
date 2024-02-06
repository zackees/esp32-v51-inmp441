
#ifndef _I2S_DEVICE_H_
#define _I2S_DEVICE_H_

#include "defs.h"

#if AUDIO_BIT_RESOLUTION == 16 && AUDIO_CHANNELS == 1
typedef int16_t audio_sample_t;
#else
#error "Unsupported audio configuration"
#endif

typedef audio_sample_t audio_buffer_t[IS2_AUDIO_BUFFER_LEN];

void i2s_audio_init();
void i2s_audio_shutdown();
size_t i2s_read_raw_samples(audio_sample_t (&buffer)[IS2_AUDIO_BUFFER_LEN]);

#endif // _I2S_DEVICE_H_
