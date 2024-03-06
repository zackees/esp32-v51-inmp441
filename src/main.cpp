/*

*/

#include <Arduino.h>

#include "math.h"

#include "main.h"
#include "defs.h"
#include "i2s_device.h"
#include "esp_pm.h"
#include <iomanip>
#include "util.h"

#include "esp_idf_version.h"

#define SLEEP_TIME_MS 2000
#define ENABLE_SLEEP 1
#define PRINT_NORMAL_OPERATION 1


#if ESP_IDF_VERSION_MAJOR > 5 || (ESP_IDF_VERSION_MAJOR == 5 && ESP_IDF_VERSION_MINOR >= 1)
// Your code here for versions 5.1 and above
  #define V5_PLUS 1
#else
// Your code here for versions below 5.1
  #define V5_PLUS 0
#endif

esp_pm_lock_handle_t apb_lock;



void enable_ledc_light_sleep() {
  #if V5_PLUS
  esp_err_t err = esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_ON);
  #else
  esp_err_t err = esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON);
  #endif
  if (err != ESP_OK) {
    Serial.printf("Failed to enable light sleep: %d\n", err);
  }
}

void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.begin(115200);

  #if ENABLE_LEDC_LIGHT_SLEEP
  ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON));
  #endif
  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
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
    size_t samples_read = i2s_read_samples(buffer, 0);
    // cout << "Samples read: " << samples_read << endl;
    //Serial.printf("Samples read: %d\n", samples_read);

    if (samples_read > 0) {
      //uint32_t start_perf = esp_log_early_timestamp();
      uint32_t diff = millis() - start_time;
      audio_sample_t* begin = &buffer[0];
      audio_sample_t* end = &buffer[samples_read];
      int32_t vol = max_volume(begin, end);
      Serial.printf("%d: ", diff);
      print_17_bitstring(vol);
      //uint32_t end_perf = esp_log_early_timestamp();
      //uint32_t perf_diff = end_perf - start_perf;
      // cout << "Performance: " << perf_diff << endl;
      //Serial.printf("Performance: %d\n", perf_diff);
    }

    //delay(1);
  }
}


void test_i2s_read_and_light_sleep() {
  static bool s_initialized = false;
  if (!s_initialized) {
    Serial.printf("Initializing i2s\n");
    s_initialized = true;
    i2s_audio_init();
  }
  cout << "Testing i2s read and light sleep\n";

  uint32_t timeout = millis() + 500;
  cout << "Normal Operation\n";

  while (timeout > millis()) {
    // uint32_t counter = i2s_get_dbg_counter();
    audio_buffer_t buffer = {0};
    audio_sample_t* begin = &buffer[0];
    // cout << "Number of reading sample capacity: " << sizeof(buffer) / sizeof(*begin) << endl;
    size_t n_samples = i2s_read_samples(buffer, 0);
    //cout << "Number of samples read: " << n_samples << endl;
    if (n_samples) {
    //Serial.printf("read %d samples\n", n_samples);
      int32_t vol = max_volume(begin, begin + n_samples);
      if (PRINT_NORMAL_OPERATION) {
        print_17_bitstring(vol);
      }
      delay(5);
    }
  }
  // Do the light sleep
  cout << "Entering light sleep\n";
  std::flush(cout);
  {
    esp_err_t err = esp_sleep_enable_timer_wakeup(2 * 1000);
    delay(50);
    if (err != ESP_OK) {
      Serial.printf("Light sleep failed: %d\n", err);
    }
    err = esp_light_sleep_start();
    if (err != ESP_OK) {
      if (err == ESP_ERR_SLEEP_REJECT) {
        Serial.printf("Light sleep failed: rejected\n");
      } else {
        Serial.printf("Light sleep failed: %d\n", err);
      }
    }
  }

  cout << "Exiting light sleep\n";
  test_microphone_distortion(15);
  cout << "tested microphone distortion\n";
}


// the loop function runs over and over again forever
void loop()
{
  //test_audio_and_i2s();
  //test_is2_and_psuedo();
  //test_i2s_isr_and_light_sleep();
  test_i2s_read_and_light_sleep();
}
