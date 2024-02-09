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
#include "defs.h"
#include "i2s_device.h"

#define SLEEP_TIME_MS 1
#define ENABLE_SLEEP 0

void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.begin(115200);
  i2s_audio_init();

  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
}

void my_light_sleep(uint32_t duration_ms) {
  #if ENABLE_SLEEP
  esp_err_t err = esp_sleep_enable_timer_wakeup(duration_ms * 1000);
  if (err != ESP_OK) {
    Serial.printf("Light sleep failed: %d\n", err);
  }
  //i2s_audio_shutdown();
  i2s_audio_enter_light_sleep();
  err = esp_light_sleep_start();
  if (err != ESP_OK) {
    if (err == ESP_ERR_SLEEP_REJECT) {
      Serial.printf("Light sleep failed: rejected\n");
    } else {
      Serial.printf("Light sleep failed: %d\n", err);
    }
  }
  i2s_audio_exit_light_sleep();
  //i2s_audio_init();
  #else
  Serial.printf("Light sleep disabled\n");
  #endif
}

int32_t max_volume(audio_sample_t* begin, audio_sample_t* end) {
  audio_sample_t* low = std::min_element(begin, end);
  audio_sample_t* high = std::max_element(begin, end);
  return *high - *low;
}


void i2s_sleep_test_microphone_distortion() {
  // Play test sound for one second, then enter light sleep mode for 1 ms, then output
  // the microphone for 1 second.
  Serial.printf("Playing test sound for one second, then entering light sleep for one second, then outputting dB sound levels for one second.\n");
  uint32_t end_time = millis() + 1000ul;
  uint32_t start_time = millis();
  while (millis() < end_time) {
    audio_buffer_t buffer = {0};
    size_t bytes_read = i2s_read_samples(buffer);
    if (bytes_read > 0) {
      uint32_t diff = millis() - start_time;
      audio_sample_t* begin = &buffer[0];
      audio_sample_t* end = &buffer[bytes_read];
      int32_t vol = max_volume(begin, end);
      Serial.printf("%d: max-min: %d, %d bytes read\n", diff, vol, bytes_read);
    }
  }
  Serial.printf("Now entering light sleep\n");
  my_light_sleep(SLEEP_TIME_MS);
  Serial.printf("Exited light sleep, now outputting dB sound levels for one second.\n");
  end_time = millis() + 1000ul;
  start_time = millis();
  while (millis() < end_time) {
    audio_buffer_t buffer = {0};
    size_t bytes_read = i2s_read_samples(buffer);
    if (bytes_read > 0) {
      uint32_t diff = millis() - start_time;
      audio_sample_t* begin = &buffer[0];
      audio_sample_t* end = &buffer[bytes_read];
      audio_sample_t* low = std::min_element(begin, end);
      audio_sample_t* high = std::max_element(begin, end);
      //Serial.printf("max: %d, min: %d\n", *high, *low);
      std::cout << "max: " << *high << ", min: " << *low << std::endl;
      int32_t vol = max_volume(begin, end);
      //Serial.printf("%d: max-min: %d, %d samples read\n", diff, vol, bytes_read);
      std::cout << diff << ": max-min: " << vol << ", " << bytes_read << " samples read" << std::endl;
    }
  }
}

// the loop function runs over and over again forever
void loop()
{
  i2s_sleep_test_microphone_distortion();
}
