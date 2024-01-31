#include "button.h"

#include <Arduino.h>
#include "defs.h"

void button_setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

bool button_is_pressed() {
  return digitalRead(BUTTON_PIN) == LOW;
}