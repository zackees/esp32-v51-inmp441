#include "low_power.h"

#include <Arduino.h>
#include "audio.h"
#include "esp_sleep.h"
#include "defs.h"

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void light_sleep(uint32_t duration_ms) {
  esp_sleep_enable_timer_wakeup(duration_ms * 1000);
  audio_enter_light_sleep();
  esp_light_sleep_start();
  audio_exit_light_sleep();
}

bool low_power_is_user_shutdown(bool is_button_pressed) {
  static uint32_t s_last_time = millis();
  static uint64_t time_on = 0;  // No wrap around.
  uint32_t now = millis();
  uint32_t diff = now - s_last_time;
  s_last_time = now;
  if (is_button_pressed) {
    time_on += diff;
  } else {
    time_on = 0;
  }
  return time_on > TIME_TURNOFF;
}

bool low_power_auto_shutoff(bool is_button_pressed) {
  static uint32_t s_last_time = millis();
  static uint64_t time_no_button = 0;  // No wrap around.
  uint32_t now = millis();
  uint32_t diff = now - s_last_time;
  s_last_time = now;
  if (!is_button_pressed) {
    time_no_button += diff;
  } else {
    time_no_button = 0;
  }
  return time_no_button > MAX_ON_TIME_NO_BUTTON_PRESS_MS;
}


void hibernate() {
  //esp_sleep_enable_ext0_wakeup((gpio_num_t)wakeup_pin, LOW);
  //rtc_gpio_pulldown_en(wakeup_pin);
  uint64_t pin_mask = 1 << BUTTON_PIN;
  esp_err_t err = esp_deep_sleep_enable_gpio_wakeup(pin_mask, ESP_GPIO_WAKEUP_GPIO_LOW);
  //esp_sleep_enable_ext0_wakeup(wakeup_pin, 1);
  if (err) {
    Serial.printf("Error setting wakeup pin: %d\n", err);
    return;
  }
  esp_deep_sleep_start();
}

