/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

#include <Arduino.h>

#include "math.h"

#include "main.h"
#include "led.h"
#include "defs.h"
#include "i2s_device.h"
#include "task.h"
#include "util.h"
#include "button.h"
#include "low_power.h"

#define MICORPHONE_CHECK_TIME 300
#define N_AUDIO_dB_HISTORY 32

void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  button_setup();
  led_setup();
  i2s_audio_init();

  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
}

void my_light_sleep(uint32_t duration_ms) {
  esp_sleep_enable_timer_wakeup(duration_ms * 1000);
  i2s_audio_shutdown();
  esp_light_sleep_start();
  i2s_audio_init();
}


void i2s_sleep_test_microphone_distortion() {
  // Play test sound for one second, then enter light sleep mode for 1 ms, then output
  // the microphone for 1 second.
  Serial.printf("Playing test sound for one second, then entering light sleep for one second, then outputting dB sound levels for one second.\n");
  uint32_t end_time = millis() + 1000ul;
  uint32_t start_time = millis();
  while (millis() < end_time) {
    audio_buffer_t buffer = {0};
    size_t bytes_read = i2s_read_raw_samples(buffer);
    if (bytes_read > 0) {
      uint32_t diff = millis() - start_time;
      audio_sample_t* begin = &buffer[0];
      audio_sample_t* end = &buffer[bytes_read];
      int16_t* low = std::min_element(begin, end);
      int16_t* high = std::max_element(begin, end);
      int16_t vol = *high - *low;
      Serial.printf("%u: dB: %d\n", diff, vol);
    }
  }
  Serial.printf("Now entering light sleep\n");
  my_light_sleep(1);
  Serial.printf("Exited light sleep, now outputting dB sound levels for one second.\n");
  end_time = millis() + 1000ul;
  start_time = millis();
  while (millis() < end_time) {
    audio_buffer_t buffer = {0};
    size_t bytes_read = i2s_read_raw_samples(buffer);
    if (bytes_read > 0) {
      uint32_t diff = millis() - start_time;
      audio_sample_t* begin = &buffer[0];
      audio_sample_t* end = &buffer[bytes_read];
      int16_t* low = std::min_element(begin, end);
      int16_t* high = std::max_element(begin, end);
      int16_t vol = *high - *low;
      Serial.printf("%u: dB: %d\n", diff, vol);
    }
  }
}

// the loop function runs over and over again forever
void loop()
{
  i2s_sleep_test_microphone_distortion();
}
