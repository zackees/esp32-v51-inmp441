/*

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

  //acquire_apb_power_lock();
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



void test_microphone_distortion(uint32_t duration_ms = 50ul) {
  uint32_t end_time = millis() + duration_ms;
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
      int32_t tmp = vol;
      char volBinary[17]; // 32 for binary digits + 1 for null terminator
      memset(volBinary, '0', 17); // Fill with '0's initially
      volBinary[17] = '\0'; // Null-terminate the string
      
      // Convert vol to binary string stored in volBinary
      for (int i = 17; i >= 0; --i) {
          volBinary[i] = (tmp & 1) + '0'; // Set the ith bit
          tmp >>= 1; // Shift vol right by 1
      }

      // Print out the information
      std::cout << std::setfill(' ') << std::setw(4) << diff << ":" << std::setw(5) << vol << " (" << volBinary << ")" << std::endl;
      delay(10);
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

void test_i2s_isr() {
  i2s_audio_init();
  while (true) {
    audio_buffer_t buffer = {0};
    audio_sample_t* begin = &buffer[0];
    audio_sample_t* end = begin + ARRAY_SIZE(buffer);
    size_t n_samples = i2s_read_samples(begin, end);
    Serial.printf("read %d samples\n", n_samples);
    delay(8);
  }
}


void test_i2s_isr_read_volume() {
  static bool s_initialized = false;
  if (!s_initialized) {
    s_initialized = true;
    i2s_audio_init();
  }

  uint32_t timeout = millis() + 500;

  while (timeout > millis()) {
    // uint32_t counter = i2s_get_dbg_counter();
    audio_buffer_t buffer = {0};
    audio_sample_t* begin = &buffer[0];
    audio_sample_t* end = begin + ARRAY_SIZE(buffer);
    size_t n_samples = i2s_read_samples(begin, end);
    if (n_samples) {
    //Serial.printf("read %d samples\n", n_samples);
      int32_t vol = max_volume(begin, begin + n_samples);

      int32_t tmp = vol;
      char volBinary[17]; // 32 for binary digits + 1 for null terminator
      memset(volBinary, '0', 17); // Fill with '0's initially
      volBinary[17] = '\0'; // Null-terminate the string
      for (int i = 17; i >= 0; --i) {
          volBinary[i] = (tmp & 1) + '0'; // Set the ith bit
          tmp >>= 1; // Shift vol right by 1
      }
      std::cout << std::setfill(' ') << std::setw(5) << vol << " (" << volBinary << ")" << std::endl;

    }
 
    // Serial.printf("max-min: %d\n", vol);
    delay(6);
  }

  // go to sleep
  my_light_sleep(1);
  test_microphone_distortion(500);
  // do noise test

}

#if 1
void test_i2s_isr_and_light_sleep() {
  i2s_audio_init();
  while (true) {
    audio_buffer_t buffer = {0};
    audio_sample_t* begin = &buffer[0];
    audio_sample_t* end = begin + ARRAY_SIZE(buffer);
    delay(1000);
    size_t n_samples = i2s_read_samples(begin, end);
    Serial.printf("read %d samples\n", n_samples);
    esp_err_t err = esp_sleep_enable_timer_wakeup(1 * 500);
    std::cout << "esp_sleep_enable_timer_wakeup: " << err << std::endl;
    if (err != ESP_OK) {
      Serial.printf("Light sleep failed: %d\n", err);
    }
    std::cout << "esp_light_sleep_start: " << err << std::endl;
    i2s_audio_enter_light_sleep();
    err = esp_light_sleep_start();
    std::cout << "esp_light_sleep_exited: " << err << std::endl;
    i2s_audio_exit_light_sleep();
    // invoke the distortion test
    test_microphone_distortion(1000);
    if (err != ESP_OK) {
      if (err == ESP_ERR_SLEEP_REJECT) {
        Serial.printf("Light sleep failed: rejected\n");
      } else {
        Serial.printf("Light sleep failed: %d\n", err);
      }
    }
  }
}
#endif  // note to test this


// the loop function runs over and over again forever
void loop()
{
  //test_audio_and_i2s();
  //test_is2_and_psuedo();
  test_i2s_isr_and_light_sleep();
  //test_i2s_isr_read_volume();
}
