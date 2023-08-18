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

void led_ramp_test()
{
  cout << "LED ramp test\n";
  for (int i = 0; i <= 255; i += 1)
  {
    if (button_is_pressed())
    {
      led_write(255);
    }
    else
    {
      led_write(i);
    }
    delay(3);
  }
  for (int i = 255; i >= 0; i -= 1)
  {
    if (button_is_pressed())
    {
      led_write(255);
    }
    else
    {
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
  cout << "Sound history buffer initialized\n";
  // Startup led sequence.
  led_ramp_test();
  cout << "LED sync animation power ramp\n";
  // set alarm to fire every 0.1 second
  cout << "Initialized\n";
}

void button_test()
{
  cout << "button test\n";
  if (button_is_pressed())
  {
    led_write(255);
  }
}

void audio_test()
{
  cout << "audio test\n";
  uint32_t expired_time = millis() + 5000ul;
  uint32_t last_audio_update_time = 0;
  while (millis() < expired_time)
  {
    audio_state_t audio_state = audio_update();
    bool has_update = audio_state.updated_at != last_audio_update_time;
    if (!has_update)
    {
      continue;
    }
    if (audio_state.dB > 70.0f || button_is_pressed())
    {
      led_write(255);
    }
    else
    {
      led_write(0);
    }
  }
}

void low_power_test()
{
  cout << "low power test\n";
  uint32_t start_button_press_time = millis();
  uint32_t timetout = 1000ul;
  if (button_is_pressed())
  {
    cout << "Button is pressed, waiting one second to test again\n";
    delay(1000);
    if (button_is_pressed())
    {
      cout << "Doing a light sleep then checking microphone" << endl;
      light_sleep(1);
      cout << "Now doing a microphone check. In a broken state the IS2 will read high values (>70 dB) before settling down to ambient noise levels.\n";
      uint32_t start_time_mic_check = millis();
      uint32_t last_audio_update_time = 0;
      while (true)
      {
        uint32_t diff = millis() - start_time_mic_check;
        if (diff > MICORPHONE_CHECK_TIME)
        {
          break;
        }
        audio_state_t audio_state = audio_update();
        bool has_update = audio_state.updated_at != last_audio_update_time;
        // print the dB of the audio
        cout << "dB: " << audio_state.dB << endl;
      }
    }
  }
  else
  {
    cout << "Button is not pressed, exiting low power test\n";
  }
}

// the loop function runs over and over again forever
void loop()
{
  led_ramp_test();
  button_test();
  audio_test();
  low_power_test();
}
