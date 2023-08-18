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
#include "buffer.hpp"
#include "led_driver.h"
#include "button.h"
#include "low_power.h"

#define N_AUDIO_dB_HISTORY 32
// #define ENABLE_LOW_POWER_MODE 1
Buffer<float> g_dB_buffer; // Buffer of

void led_sync_animation_power_ramp()
{
  for (int i = 0; i <= 255; i += 1)
  {
    if (button_is_pressed())
    {
      led_write(255);
    } else {
      led_write(i);
    }
    delay(3);
  }
  for (int i = 255; i >= 0; i -= 1)
  {
    if (button_is_pressed())
    {
      led_write(255);
    } else {
      led_write(i);
    }
    delay(3);
  }
}

void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  button_setup();
  cout << "Button initialized\n";
  led_setup();
  cout << "LED initialized\n";
  audio_init();
  cout << "Audio initialized\n";
  g_dB_buffer.init(N_AUDIO_dB_HISTORY);
  cout << "Sound history buffer initialized\n";
  // Startup led sequence.
  led_sync_animation_power_ramp();
  cout << "LED sync animation power ramp\n";
  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
}


// the loop function runs over and over again forever
void loop()
{
  // test 1
  led_sync_animation_power_ramp();
  if (button_is_pressed())
  {
    led_write(255);
  }
  uint32_t expired_time = millis() + 5000ul;
  while (millis() < expired_time)
  {
    audio_state_t audio_state = audio_update();
    if (audio_state.dB > 70.0f || button_is_pressed())
    {
      led_write(255);
    } else {
      led_write(0);
    }
  }
  uint32_t start_button_press_time = millis();
  uint32_t timetout = 1000ul;
  if (button_is_pressed())
  {
    delay(1000);
    if (button_is_pressed()) {
      cout << "Doing a light sleep then checking microphone" << endl;
      light_sleep(1);
      uint32_t start_time_mic_check = millis();
      while (true) {
        uint32_t diff = millis() - start_time_mic_check;
        if (diff > 1000) {
          break;
        }
        audio_state_t audio_state = audio_update();
        // print the dB of the audio
        cout << "dB: " << audio_state.dB << endl;
      }
    }
  }
}
