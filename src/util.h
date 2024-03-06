#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <cmath>

inline float linear_interpolation(float start_value, float end_value, float t) {
    return start_value + (end_value - start_value) * t;
}

inline float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <typename T>
inline T clamp(T x, T min, T max) {
  if (x < min) {
    return min;
  } else if (x > max) {
    return max;
  } else {
    return x;
  }
}

inline float mapf_clamped(float x, float in_min, float in_max, float out_min, float out_max) {
  return clamp<float>(mapf(x, in_min, in_max, out_min, out_max), out_min, out_max);
}

// represents A = pe^(r*t) function.
// Allows continuous integration.
inline float exponential_interpolation(float x, float rate, float time) {
  return x * std::exp(rate * time);
}

uint64_t millis64();
char pixelBrightnessToChar(float value, float min_value, float max_value);

void print_17_bitstring(int32_t vol);

#endif  // _UTIL_H_