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
#define LEDC_DUTY_RES LEDC_TIMER_4_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY (1024)           // Frequency in Hertz. Set frequency at 5 kHz
#define PIN_PSUEDO_I2S GPIO_NUM_6

// #define LEDC_CLOCK LEDC_USE_RC_FAST_CLK  // still clocks during light sleep.
#define LEDC_CLOCK LEDC_USE_RTC8M_CLK  // still clocks during light sleep.

namespace
{
  enum {
    // use bith manipuation against the LEDC_TIMER_8_BIT to come to a max duty.
    // For example, if LEDC_TIMER_8_BIT is 8, then the max duty is 255.
    kMaxDuty = (1 << LEDC_TIMER_8_BIT) - 1,
  };
  ledc_timer_config_t ledc_timer = {
      //.duty_resolution  = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .speed_mode = LEDC_MODE,
      .duty_resolution = LEDC_DUTY_RES,
      .timer_num = LEDC_TIMER,
      .freq_hz = LEDC_FREQUENCY, // Set output frequency
      .clk_cfg = LEDC_CLOCK
    };

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {
      .gpio_num = PIN_PSUEDO_I2S,
      .speed_mode = LEDC_MODE,
      .channel = LEDC_CHANNEL,
      .intr_type = LEDC_INTR_FADE_END,
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
  //rtc_clk_slow_freq_set(RTC_SLOW_FREQ_8MD256);
  std::cout << "pseudo_i2s_start\n";
  std::flush(std::cout);
  ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON));
  ESP_ERROR_CHECK(gpio_sleep_sel_dis(PIN_PSUEDO_I2S)); // Needed for light sleep.
  // Prepare and then apply the LEDC PWM timer configuration
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, kMaxDuty / 2));
  std::cout << "pseudo_i2s_start done\n";
  std::flush(std::cout);
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
  std::cout << "pseudo_i2s_start done\n";
  std::flush(std::cout);

}

void pseudo_i2s_stop()
{
  ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0));
  pinMode(PIN_PSUEDO_I2S, INPUT);
}