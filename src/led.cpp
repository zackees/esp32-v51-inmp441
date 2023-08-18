#include "led.h"
#include <math.h>
#include <Arduino.h>


#include "driver/ledc.h"
#include "defs.h"

#define MAX_8BIT 255
#define MAX_16BIT 65535
#define MAX_LED_VALUE  ((1 << LED_PWM_RESOLUTION) - 1)

// #define ENABLE_SLEEP_COMPATIBLE_LEDC

namespace {
  uint16_t gamma_correct(uint8_t val) {
    return uint16_t(val) * uint16_t(val);
  }

  uint32_t map_to_led_duty_cycle(uint16_t val16) {
    uint32_t val32 = map(val16, 0, MAX_16BIT, 0, MAX_LED_VALUE);
    return val32;
  }
}



void ledcWrite_custom(uint8_t chan, uint32_t duty)
{
#ifdef ENABLE_SLEEP_COMPATIBLE_LEDC
    uint8_t group=(chan/8), channel=(chan%8);

    //Fixing if all bits in resolution is set = LEDC FULL ON
    uint32_t max_duty = (1 << LED_PWM_RESOLUTION) - 1;

    if((duty == max_duty) && (max_duty != 1)){
        duty = max_duty + 1;
    }

    ledc_set_duty(ledc_mode_t(group), ledc_channel_t(channel), duty);
    ledc_update_duty(ledc_mode_t(group), ledc_channel_t(channel));
#else
    ledcWrite(chan, duty);
#endif
}


void led_setup() {
  ledcAttachPin(LED_PIN, LED_0);
  ledcSetup(LED_0, LED_PWM_FREQ, LED_PWM_RESOLUTION);
  #ifdef ENABLE_SLEEP_COMPATIBLE_LEDC
  uint8_t group=(LED_0/8), timer=((LED_0/2)%4);
  ledc_timer_config_t ledc_timer = {
      .speed_mode       = ledc_mode_t(group),
      .duty_resolution  = ledc_timer_bit_t(LED_PWM_RESOLUTION),
      .timer_num        = ledc_timer_t(timer),
      .freq_hz          = LED_PWM_FREQ,
      //.clk_cfg          = LEDC_DEFAULT_CLK
      .clk_cfg          = LEDC_USE_RTC8M_CLK  // Allows LEDC to work when CPU is in sleep mode
  };
  if(ledc_timer_config(&ledc_timer) != ESP_OK)
  {
      log_e("ledc setup failed!");
      return;
  }
  //channels_resolution[chan] = bit_num;
  //return ledc_get_freq(group,timer);
  #endif
  led_write(0);
}

void led_write(uint8_t val) {
  uint16_t val16 = gamma_correct(val);
  uint32_t val32 = map_to_led_duty_cycle(val16);
  #if LED_PIN_IS_SINK
  static const uint32_t kMaxVal32 = map_to_led_duty_cycle(MAX_16BIT);
  val32 = kMaxVal32 - val32;
  #endif
  ledcWrite_custom(LED_0, val32);
}

void test_led() {
  const int delay_ms = 16;
  for (int i = 0; i <= 255; i += 1) {
    led_write(i);
    delay(delay_ms);
  }
  for (int i = 255; i >= 0; i -= 1) {
    led_write(i);
    delay(delay_ms);
  }
}
