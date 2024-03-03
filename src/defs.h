#ifndef _DEFS_H_
#define _DEFS_H_

#include <iostream>
using std::cout;
using std::endl;

#define MAX_ANALOG_READ 1023
#define MAX_ON_TIME_NO_BUTTON_PRESS_MS 1000 * 60 * 60 * 4  // 4 hours

#define BUTTON_PIN GPIO_NUM_2

// GPIO18 and GPIO19 are held high when USB is connected to the host computer.

// Note that on the original board LED pin is pin 9.
#define LED_PIN GPIO_NUM_9  // Pin is held high on startup.
#define LED_PIN_IS_SINK true
#define LED_0 LEDC_CHANNEL_0  // Led channel name
#define LED_PWM_FREQ 2000
#define LED_PWM_RESOLUTION 14  // Max on ESP32-c3 XIOA

#define TIME_PWM_CYCLE_MS  3 // Flickers at 1ms
#define TIME_PWM_TRANSITION_MS 3  // 60 fps

#define PIN_I2S_WS GPIO_NUM_7
#define PIN_IS2_SD GPIO_NUM_8
#define PIN_I2S_SCK GPIO_NUM_4
#define PIN_AUDIO_PWR GPIO_NUM_10

#define I2S_NUM I2S_NUM_0
#define IS2_SAMPLES_PER_CHANNEL 16
#define AUDIO_BIT_RESOLUTION 16
#define AUDIO_CHANNEL_SAMPLE_RATE (48000ul)
#define AUDIO_SAMPLE_RATE (AUDIO_CHANNEL_SAMPLE_RATE * AUDIO_CHANNELS)
#define AUDIO_CHANNELS 1 // Not tested with 2 channels
#define AUDIO_SAMPLES_PER_DMA_BUFFER 16
#define AUDIO_DMA_BUFFER_COUNT 64
#define AUDIO_RECORDING_SECONDS 1

#define LIGHT_SLEEP_TIME_uS uint32_t(1000 * 100) // 100 ms.
#define TIME_TURNOFF 1500 // 1 second

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define ASSERT_IMPL(x, msg, file, lineno)                                                                       \
  do                                                                                                            \
  {                                                                                                             \
    if (!(x))                                                                                                   \
    {                                                                                                           \
      std::cout << "#############\n# ASSERTION FAILED: " << file << ":" << lineno << "\n# MSG: " << msg << "\n#############\n"; \
      configASSERT(x);                                                                                          \
    }                                                                                                           \
  } while (false);

#define ASSERT(x, msg) ASSERT_IMPL(x, msg, __FILE__, __LINE__)


#endif  // _DEFS_H_