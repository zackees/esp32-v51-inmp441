#include <Arduino.h>
#include "led_driver.h"

#include "defs.h"
#include "led.h"
#include "task.h"
  #include <cmath>
#include <atomic>
#include "util.h"
#include <iostream>

using namespace std;

#define DEFAULT_MIN_DB 67.f
#define DEFAULT_MAX_DB 80.f

#define LED_DECAY_RATE -0.01f
#define LED_ATTACK_RATE 0.1f

namespace {
  uint8_t s_curr_led = 0;
  float s_curr_db = 0.f;
  float s_prev_vol = 0.f;
  float s_min_db = DEFAULT_MIN_DB;
  float s_max_db = DEFAULT_MAX_DB;


  void led_driver() {
    while (true) {
      led_driver_update(8);
    }
  }
}

void led_driver_setup() {
  
}

void led_driver_set_window(float min_db, float max_db) {
  s_min_db = min_db;
  s_max_db = max_db;
}

void led_driver_update_dB(float db) {
  s_curr_db = db;
}

uint8_t led_driver_update(uint32_t duration_ms) {
    float time = float(duration_ms) / TIME_PWM_CYCLE_MS;
    float vol = mapf_clamped(s_curr_db, s_min_db, s_max_db, 0.f, 1.f);
    float next_vol = 0.f;
    if (vol < s_prev_vol) {
      float decay_rate = LED_DECAY_RATE;
      if (s_prev_vol < .2f) {
        decay_rate *= mapf_clamped(s_prev_vol, .2f, 0.f, 1.f, 0.f);
      }
      next_vol = s_prev_vol * std::exp(decay_rate * time);
    } else {
      float maybe_next_vol = s_prev_vol * std::exp(LED_ATTACK_RATE * time);
      if (maybe_next_vol > s_prev_vol && maybe_next_vol > 0.01f) {  // Need a boost to get over the hump.
        next_vol = clamp(vol, s_prev_vol, maybe_next_vol);
      } else {
        next_vol = vol;
      }
    }
    uint8_t led_value = uint8_t(next_vol * 255);
    // cout << "led_value: " << int(led_value) << "\n";
    //led_write(led_value);
    s_prev_vol = next_vol;
    s_curr_led = led_value;
    //return led_value > 0;
    return led_value;
}