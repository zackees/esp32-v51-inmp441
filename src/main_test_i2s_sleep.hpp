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
#include "audio.h"
#include "task.h"
#include "util.h"
#include "led_driver.h"
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
  audio_init();

  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
}


void i2s_sleep_test_microphone_distortion() {
  // Play test sound for one second, then enter light sleep mode for 1 ms, then output
  // the microphone for 1 second.
  Serial.printf("Playing test sound for one second, then entering light sleep for one second, then outputting dB sound levels for one second.\n");
  uint32_t end_time = millis() + 1000ul;
  uint32_t last_update_time = 0;
  uint32_t start_time = millis();
  while (millis() < end_time) {
    audio_state_t audio_state = audio_update();
    if (audio_state.updated_at > last_update_time) {
      last_update_time = audio_state.updated_at;
      uint32_t diff = millis() - start_time;
      Serial.printf("%u: dB: %d\n", diff, int(audio_state.dB));
    }
  }
  Serial.printf("Now entering light sleep\n");
  light_sleep(1);
  Serial.printf("Exited light sleep, now outputting dB sound levels for one second.\n");
  end_time = millis() + 1000ul;
  last_update_time = 0;
  start_time = millis();
  while (millis() < end_time) {
    audio_state_t audio_state = audio_update();
    if (audio_state.updated_at > last_update_time) {
      last_update_time = audio_state.updated_at;
      uint32_t diff = millis() - start_time;
      Serial.printf("%u: dB: %d\n", diff, int(audio_state.dB));
    }
  }
}

// the loop function runs over and over again forever
void loop()
{
  i2s_sleep_test_microphone_distortion();
}
