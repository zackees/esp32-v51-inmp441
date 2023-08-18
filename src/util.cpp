#include "util.h"

#include <Arduino.h>

uint64_t millis64()
{
  // NOT thread safe.
  static uint32_t last_millis = 0;
  static uint64_t accumulator = 0;
  uint32_t now = millis();
  uint32_t diff = now - last_millis; // works even for wrap around.
  accumulator += diff;
  last_millis = now;
  return accumulator;
}

char pixelBrightnessToChar(float value, float min_value, float max_value)
{
  // Ensure the input is in the expected range
  if (value < min_value)
    value = min_value;
  if (value > max_value)
    value = max_value;

  const char *gradation = " .:-=+*%@#";
  size_t levels = strlen(gradation);

  // Determine the index into the gradation string
  size_t index = (size_t)((value - min_value) / (max_value - min_value) * (levels - 1));

  return gradation[index];
}