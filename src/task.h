#ifndef _TASK_H_
#define _TASK_H_

#include <Arduino.h>

inline void delay_task_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

#endif  // _TASK_H_
