#include "status_led.h"
#include "config.h"

#include <Arduino.h>

namespace {

inline void writeLed(bool on) {
#if STATUS_LED_ACTIVE_LOW
  digitalWrite(STATUS_LED_PIN, on ? LOW : HIGH);
#else
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
#endif
}

}  // namespace

void statusLedBegin() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  statusLedOff();
}

void statusLedOn() {
  writeLed(true);
}

void statusLedOff() {
  writeLed(false);
}
