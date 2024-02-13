

#include <math.h>
#include <Arduino.h>
#include <stdio.h>

#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "defs.h"
#include "pseudo_i2s.h"

#define MAX_8BIT 255
#define MAX_16BIT 65535
#define MAX_LED_VALUE ((1 << LED_PWM_RESOLUTION) - 1)


#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_1
#define LEDC_DUTY_RES LEDC_TIMER_2_BIT // Set duty resolution to 13 bits
//#define LEDC_DUTY (4095)                // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FULL_DUTY (16383)            // Set duty to 100%. ((2 ** 14) - 1) = 16383
#define LEDC_FREQUENCY (1024*5)           // Frequency in Hertz. Set frequency at ~5 kHz

#define PIN_PSUEDO_I2S GPIO_NUM_21
#define LEDC_CLOCK LEDC_USE_RC_FAST_CLK  // still clocks during light sleep.


namespace
{
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
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER,
      .duty = 1,
      .hpoint = 2};
} // namespace

void pseudo_i2s_start()
{
  // Prepare and then apply the LEDC PWM timer configuration
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void pseudo_i2s_stop()
{
  ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0));
}