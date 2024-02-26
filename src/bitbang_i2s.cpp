

#include "bitbang_i2s.h"

#include "Arduino.h"
#include "defs.h"

#define SCK_HIGH_NS 50
#define SCK_LOW_NS 50
#define WS_HOLD_NS 20
#define WS_SETUP_NS 0

/*
#define PIN_I2S_WS GPIO_NUM_7
#define PIN_IS2_SD GPIO_NUM_8
#define PIN_I2S_SCK GPIO_NUM_4
#define PIN_AUDIO_PWR GPIO_NUM_10

#define PIN_WS GPIO_NUM_21
#define PIN_SCK GPIO_NUM_6
#define PIN_SD PIN_IS2_SD
*/

#define PIN_WS PIN_I2S_WS
#define PIN_SCK PIN_I2S_SCK
#define PIN_SD PIN_IS2_SD

void bit_bang_setup()
{
  pinMode(PIN_WS, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_SD, INPUT);
  pinMode(PIN_AUDIO_PWR, OUTPUT);
  digitalWrite(PIN_AUDIO_PWR, HIGH); // Power on the IS2 microphone.
}

// Utility function to delay for a number of nanoseconds using ESP32's tick counter
void delayNanoseconds(uint32_t count)
{
  for (int i = 0; i < count; ++i)
  {
    asm("nop");
  }
}

void bit_bang_get_sample(uint16_t *sample)
{
  // Ensure pin modes are set
  // Initialize the sample
  *sample = 0;
  while (true)
  {
    // Start the WS (Word Select) pulse
    digitalWrite(PIN_WS, LOW);
    digitalWrite(PIN_SCK, LOW);  // Set the clock low
    digitalWrite(PIN_WS, HIGH);  // Set the WS high
    digitalWrite(PIN_SCK, HIGH); // Start the clock
    for (int bit = 15; bit >= -15; --bit)
    {                             // Loop through each bit of the sample
      digitalWrite(PIN_SCK, LOW); // Set the clock low
      // delayNanoseconds(SCK_LOW_NS); // Wait for half the clock period
      // Read the data bit
      if (bit >= 0)
      {
        int dataBit = digitalRead(PIN_SD) == HIGH ? 1 : 0;
        *sample |= (dataBit << bit); // Store the read bit in the sample
      }
      digitalWrite(PIN_SCK, HIGH); // Set the clock high
      // delayNanoseconds(SCK_HIGH_NS); // Wait for the other half of the clock period
    }

    // Finish the WS pulse
    digitalWrite(PIN_WS, LOW);

    // delayNanoseconds(WS_HOLD_NS); // Hold time for the word select
    if (WS_SETUP_NS > 0)
    {
      delayNanoseconds(1); // Setup time for the next word select, if needed
    }
  }
}