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
#include "esp_pm.h"
#include <iomanip>
#include "pseudo_i2s.h"

#define SLEEP_TIME_MS 2000
#define ENABLE_SLEEP 1

esp_pm_lock_handle_t apb_lock;

void acquire_apb_power_lock() {
  esp_err_t err = esp_pm_lock_create(
    ESP_PM_APB_FREQ_MAX,
    0,
    "i2s-apb-lock",
    &apb_lock);
}

void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.begin(115200);
  //i2s_audio_init();

  acquire_apb_power_lock();
  ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON));

  //esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_ON);
  //pseudo_i2s_start();

  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
}

void my_light_sleep(uint32_t duration_ms) {
  std::cout << "my_light_sleep\n";
  #if ENABLE_SLEEP
  esp_err_t err = esp_sleep_enable_timer_wakeup(duration_ms * 1000);
  std::cout << "esp_sleep_enable_timer_wakeup: " << err << std::endl;
  if (err != ESP_OK) {
    Serial.printf("Light sleep failed: %d\n", err);
  }
  //i2s_audio_shutdown();
  //i2s_audio_enter_light_sleep();
  //pseudo_i2s_start();
  std::cout << "esp_light_sleep_start: " << err << std::endl;
  err = esp_light_sleep_start();
  std::cout << "esp_light_sleep_exited: " << err << std::endl;

  if (err != ESP_OK) {
    if (err == ESP_ERR_SLEEP_REJECT) {
      Serial.printf("Light sleep failed: rejected\n");
    } else {
      Serial.printf("Light sleep failed: %d\n", err);
    }
  }
  //pseudo_i2s_stop();
  //i2s_audio_exit_light_sleep();
  //i2s_audio_init();
  #else
  Serial.printf("Light sleep disabled\n");
  #endif
}

void my_light_sleep2(uint32_t duration_ms) {
  std::cout << "my_light_sleep\n";
  #if ENABLE_SLEEP
  esp_err_t err = esp_sleep_enable_timer_wakeup(duration_ms * 1000);
  std::cout << "esp_sleep_enable_timer_wakeup: " << err << std::endl;
  if (err != ESP_OK) {
    Serial.printf("Light sleep failed: %d\n", err);
  }
  //i2s_audio_shutdown();
  //i2s_audio_enter_light_sleep();
  pseudo_i2s_start();
  std::cout << "esp_light_sleep_start: " << err << std::endl;
  err = esp_light_sleep_start();
  std::cout << "esp_light_sleep_exited: " << err << std::endl;

  if (err != ESP_OK) {
    if (err == ESP_ERR_SLEEP_REJECT) {
      Serial.printf("Light sleep failed: rejected\n");
    } else {
      Serial.printf("Light sleep failed: %d\n", err);
    }
  }
  pseudo_i2s_stop();
  //i2s_audio_exit_light_sleep();
  //i2s_audio_init();
  #else
  Serial.printf("Light sleep disabled\n");
  #endif
}

int32_t max_volume(audio_sample_t* begin, audio_sample_t* end) {
  audio_sample_t* low = std::min_element(begin, end);
  audio_sample_t* high = std::max_element(begin, end);
  return int32_t(*high) - *low;
}


void test_microphone_distortion() {
  uint32_t end_time = millis() + 1000ul;
  uint32_t start_time = millis();
  while (true) {
    uint32_t now = millis();
    if (now > end_time) {
      break;
    }
    uint32_t diff = now - start_time;
    audio_buffer_t buffer = {0};
    size_t bytes_read = i2s_read_samples(buffer);
    if (bytes_read > 0) {
      uint32_t diff = millis() - start_time;
      audio_sample_t* begin = &buffer[0];
      audio_sample_t* end = &buffer[bytes_read];
      int32_t vol = max_volume(begin, end);
      std::cout << std::setfill(' ') << std::setw(4) << diff << ":" << std::setw(5) << vol << std::endl;
    }
  }
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
      std::cout << "vol: " << vol << std::endl;
    }
  }
  Serial.printf("Now entering light sleep\n");
  my_light_sleep(SLEEP_TIME_MS);
  Serial.printf("Exited light sleep, now outputting sound levels for one second.\n");
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
      //std::cout << "max: " << *high << ", min: " << *low << std::endl;
      int32_t vol = max_volume(begin, end);
      //Serial.printf("%d: max-min: %d, %d samples read\n", diff, vol, bytes_read);
      //std::cout << diff << ": max-min: " << vol << ", " << bytes_read << " samples read" << std::endl;
      // print vol
      std::cout << "vol: " << vol << std::endl;
    }
  }
}

void test_pseudo_i2s_start() {
  delay(2000);
  // pulse the light to let me know it's startup time
  //esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
  // RTC fast

  uint32_t count = 0;

  pseudo_i2s_start();
  while (1) {
    Serial.printf("loop: %d\n", count);
    Serial.println("about to sleep");
    std::flush(std::cout);
    pseudo_i2s_start();
    my_light_sleep(2000);
    pseudo_i2s_stop();
    Serial.println("woke up");
    std::flush(std::cout);
    delay(250);
    count++;
  }
}


void test_audio_and_i2s() {
  delay(2000);
  // pulse the light to let me know it's startup time
  //esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
  // RTC fast
  uint32_t count = 0;
  i2s_audio_init();
  while (1) {
    Serial.printf("loop: %d\n", count);
    Serial.println("about to sleep");
    std::flush(std::cout);
    //i2s_audio_enter_light_sleep();
    //i2s_audio_enter_light_sleep();
    esp_err_t err = esp_sleep_enable_timer_wakeup(500 * 1000);
    i2s_audio_enter_light_sleep();
    pseudo_i2s_start();
    // Turn on built in LED for esp32 c3
    //std::cout << "esp_light_sleep_start: " << err << std::endl;
    err = esp_light_sleep_start();
    //std::cout << "esp_light_sleep_exited: " << err << std::endl;
    if (err != ESP_OK) {
      if (err == ESP_ERR_SLEEP_REJECT) {
        Serial.printf("Light sleep failed: rejected\n");
      } else {
        Serial.printf("Light sleep failed: %d\n", err);
      }
    }
    pseudo_i2s_stop();
    i2s_audio_exit_light_sleep();
    test_microphone_distortion();
    //i2s_audio_exit_light_sleep();
    //Serial.println("woke up");
    std::flush(std::cout);
    //delay(1000);
    count++;
  }
}


void test_is2_and_psuedo() {
  i2s_audio_init();
  pseudo_i2s_start();
  while (true) {
    ;
  }
}


// the loop function runs over and over again forever
void loop()
{
  test_audio_and_i2s();
  //test_is2_and_psuedo();
}
