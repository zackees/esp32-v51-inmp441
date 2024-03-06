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

void print_17_bitstring(int32_t vol) {  // diff between max and min int16_t
  int32_t tmp = vol;
  char volBinary[20] = {0}; // 32 for binary digits + 1 for null terminator
  //memset(volBinary, '0', 17); // Fill with '0's initially
  volBinary[17] = '\0'; // Null-terminate the string
  for (int i = 17; i >= 0; --i) {
      volBinary[i] = (tmp & 1)? '1' : '0'; // Set the ith bit
      tmp >>= 1; // Shift vol right by 1
  }
  // std::cout << std::setfill(' ') << std::setw(5) << vol << " (" << volBinary << ")" << std::endl;
  // Serial.printf("%d (%s)\n", vol, volBinary);
  // Correct the printf so that it behaves like the std::cout with the proper spacing
  Serial.printf("%5d (%s)\n", vol, volBinary);
}
