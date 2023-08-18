#ifndef _LOW_POWER_H_
#define _LOW_POWER_H_

#include <Arduino.h>
#include "audio.h"
#include "esp_sleep.h"
#include "defs.h"

void print_wakeup_reason();
void light_sleep(uint32_t duration_ms);
bool low_power_is_user_shutdown(bool is_button_pressed);
bool low_power_auto_shutoff(bool is_button_pressed);
void hibernate();


#endif // _LOW_POWER_H_
