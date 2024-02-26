/*
Due to a glitch in the INMP441 hardware, we have to keep the I2S clock running during
sleep or else the microphone will mix in noise for ~100ms.
This module use the LEDC PWM to keep the INMP441 clock running (preventing INMP441 sleep)
during main mcu sleep.
*/

#include <math.h>
#include <Arduino.h>
#include <stdio.h>

#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "defs.h"
#include "pseudo_i2s.h"

#include "esp32-hal-ledc.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_1_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY_SCK (1024 * 1500)  // 1 mhz clock.
#define LEDC_FREQUENCY_WS (LEDC_FREQUENCY_SCK / 32)  // 32 clocks per frame.
#define PIN_PSUEDO_I2S_SCK GPIO_NUM_6

#define PIN_PSUEDO_I2S_WS GPIO_NUM_21

// #define LEDC_CLOCK LEDC_USE_RC_FAST_CLK  // still clocks during light sleep.
#define LEDC_CLOCK LEDC_USE_RTC8M_CLK  // still clocks during light sleep.

namespace
{
  enum {
    // use bith manipuation against the LEDC_TIMER_8_BIT to come to a max duty.
    // For example, if LEDC_TIMER_8_BIT is 8, then the max duty is 255.
    kMaxDuty = (1 << LEDC_DUTY_RES) - 1,
  };
  ledc_timer_config_t ledc_timer_sck = {
      //.duty_resolution  = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .speed_mode = LEDC_MODE,
      .duty_resolution = LEDC_DUTY_RES,
      .timer_num = LEDC_TIMER,
      .freq_hz = LEDC_FREQUENCY_SCK, // Set output frequency
      .clk_cfg = LEDC_CLOCK
  };

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel_sck = {
      .gpio_num = PIN_PSUEDO_I2S_SCK,
      .speed_mode = LEDC_MODE,
      .channel = LEDC_CHANNEL,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER,
      .duty = 0,
      .hpoint = 0,
      .flags = {
        .output_invert = 1,
      }
  };

  ledc_timer_config_t ledc_timer_ws = {
      //.duty_resolution  = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .speed_mode = LEDC_MODE,
      .duty_resolution = LEDC_DUTY_RES,
      .timer_num = LEDC_TIMER_1,
      .freq_hz = LEDC_FREQUENCY_WS, // Set output frequency
      .clk_cfg = LEDC_CLOCK
  };

  ledc_channel_config_t ledc_channel_ws = {
      .gpio_num = PIN_PSUEDO_I2S_WS,
      .speed_mode = LEDC_MODE,
      .channel = LEDC_CHANNEL_1,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER,
      .duty = 0,
      .hpoint = 0,
      .flags = {
        .output_invert = 1,
      }
  };

} // namespace

void pseudo_i2s_start()
{
  ESP_ERROR_CHECK(gpio_sleep_sel_dis(PIN_PSUEDO_I2S_SCK)); // Needed for light sleep.
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_sck));
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_sck));
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 1));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

  // ws pin setup
  ESP_ERROR_CHECK(gpio_sleep_sel_dis(PIN_PSUEDO_I2S_WS)); // Needed for light sleep.
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_ws));
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_ws));
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, 1));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1));

}

void pseudo_i2s_stop()
{
  ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0));
  //pinMode(PIN_PSUEDO_I2S_SCK, OUTPUT);
  //digitalWrite(PIN_PSUEDO_I2S_SCK, LOW);
  pinMode(PIN_PSUEDO_I2S_SCK, INPUT);
}