

#include <math.h>
#include <Arduino.h>
#include <stdio.h>

#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "defs.h"
#include "pseudo_i2s.h"

// #include /Users/niteris/.platformio/packages/framework-arduinoespressif32@src-cba3def1496a47e6af73c0b73bd2e13c/cores/esp32/esp32-hal-ledc.c
#include "esp32-hal-ledc.h"
///Users/niteris/.platformio/packages/framework-arduinoespressif32@src-cba3def1496a47e6af73c0b73bd2e13c/cores/esp32/esp32-hal-periman.h
#include "esp32-hal-periman.h"

#define MAX_8BIT 255
#define MAX_16BIT 65535
#define MAX_LED_VALUE ((1 << LED_PWM_RESOLUTION) - 1)


#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_11_BIT // Set duty resolution to 13 bits
//#define LEDC_DUTY (4095)                // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FULL_DUTY (16383)            // Set duty to 100%. ((2 ** 14) - 1) = 16383
#define LEDC_FREQUENCY (1024)           // Frequency in Hertz. Set frequency at 5 kHz

#define PIN_PSUEDO_I2S GPIO_NUM_6
#define LEDC_CLOCK LEDC_USE_XTAL_CLK  // still clocks during light sleep.


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
      .intr_type = LEDC_INTR_FADE_END,
      .timer_sel = LEDC_TIMER,
      .duty = 0,
      .hpoint = 0,
      .flags = {
        .output_invert = 1,
      }
  };



static uint8_t analog_resolution = 8;
static int analog_frequency = 5000;
void myAnalogWrite(uint8_t pin, int value) {
  // Use ledc hardware for internal pins
  if (pin < SOC_GPIO_PIN_COUNT) {
    ledc_channel_handle_t *bus = (ledc_channel_handle_t*)perimanGetPinBus(pin, ESP32_BUS_TYPE_LEDC);
    if(bus == NULL && perimanClearPinBus(pin)){
        if(ledcAttach(pin, analog_frequency, analog_resolution) == 0){
            log_e("analogWrite setup failed (freq = %u, resolution = %u). Try setting different resolution or frequency");
            return;
        }
    }
    ledcWrite(pin, value);
  }
}

} // namespace



void pseudo_i2s_start()
{
  //rtc_clk_slow_freq_set(RTC_SLOW_FREQ_8MD256);
  std::cout << "pseudo_i2s_start\n";
  std::flush(std::cout);
  //pinMode(PIN_PSUEDO_I2S, OUTPUT);
  //digitalWrite(PIN_PSUEDO_I2S, HIGH);
  //return;
  myAnalogWrite(PIN_PSUEDO_I2S, 127);
  return;

  //return;
  // Prepare and then apply the LEDC PWM timer configuration
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  //myAnalogWrite(PIN_PSUEDO_I2S, 127);
  //return;
  //ESP_ERROR_CHECK(ledc_fade_func_install(0));
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128));
  std::cout << "pseudo_i2s_start done\n";
  std::flush(std::cout);
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
  std::cout << "pseudo_i2s_start done\n";
  std::flush(std::cout);
}

void pseudo_i2s_stop()
{
  //return;
  //ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0));
}