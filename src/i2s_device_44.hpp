
/*
Legacy 4.4 i2s driver.
*/


#include <iostream>

#include "defs.h"
#include <Arduino.h>
#include <stdint.h>
#include <driver/i2s.h>
#include <limits>
#include "time.h"
#include <stdio.h>
#include <atomic>
#include "driver/gpio.h"

#include "i2s_device.h"


using namespace std;

#define ENABLE_AUDIO_TASK 0

#define AUDIO_TASK_SAMPLING_PRIORITY 7

#define AUDIO_BUFFER_SAMPLES (AUDIO_RECORDING_SECONDS * AUDIO_SAMPLE_RATE * AUDIO_CHANNELS)

// During power
#define POWER_ON_TIME_MS 85  // Time to power on the microphone according to the datasheet.
#define POWER_OFF_TIME_MS 85 // Time to power off the microphone is 43 ms but we round up.
                             // Note that during power down, no data should be attempted to be read
                             // or the ESD diodes will be activated and the microphone will be damaged.

namespace
{
  static_assert(AUDIO_BIT_RESOLUTION == 16, "Only 16 bit resolution is supported");
  static_assert(AUDIO_CHANNELS == 1, "Only 1 channel is supported");
  static_assert(sizeof(audio_sample_t) == 2, "audio_sample_t must be 16 bit");

  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = AUDIO_SAMPLE_RATE,
      .bits_per_sample = i2s_bits_per_sample_t(AUDIO_BIT_RESOLUTION),
      .channel_format = i2s_channel_fmt_t(I2S_CHANNEL_FMT_ONLY_RIGHT),
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0,
      .dma_buf_count = AUDIO_DMA_BUFFER_COUNT,
      .dma_buf_len = IS2_AUDIO_BUFFER_LEN,
      //.use_apll = true
  };

  const i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_I2S_SCK,
      .ws_io_num = PIN_I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_IS2_SD};

}

void i2s_audio_init()
{

  pinMode(PIN_AUDIO_PWR, OUTPUT);
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
  // i2s_start(I2S_NUM_0);
}

void i2s_audio_shutdown()
{
  // i2s_stop(I2S_NUM_0);

  i2s_driver_uninstall(I2S_NUM_0);
}

void i2s_audio_enter_light_sleep()
{
  // digitalWrite(PIN_AUDIO_PWR, LOW); // Power off the IS2 microphone.
  // hold pin engaged
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
  gpio_hold_en(PIN_AUDIO_PWR);
  i2s_stop(I2S_NUM_0);
  //i2s_driver_uninstall(I2S_NUM_0);
}

void i2s_audio_exit_light_sleep()
{
  // digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.

  i2s_start(I2S_NUM_0);
  //i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
}

size_t i2s_read_samples(audio_sample_t (&buffer)[IS2_AUDIO_BUFFER_LEN])
{

  size_t bytes_read = 0;
  i2s_event_t event;

  uint32_t current_time = millis();
  esp_err_t result = i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytes_read, 0);
  if (result == ESP_OK)
  {
    if (bytes_read > 0)
    {
      // cout << "Bytes read: " << bytes_read << endl;
      const size_t count = bytes_read / sizeof(audio_sample_t);
      return count;
    }
  }
  return 0;
}