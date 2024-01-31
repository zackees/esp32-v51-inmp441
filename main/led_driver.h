#ifndef _LED_DRIVER_H_
#define _LED_DRIVER_H_

#include <stdint.h>

void led_driver_setup();
void led_driver_update_dB(float db);
void led_driver_set_window(float min_db, float max_db);

// Returns The current led value after the update is applied.
uint8_t led_driver_update(uint32_t duration_ms);

#endif  // _LED_DRIVER_H_
