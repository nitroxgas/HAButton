#include "serial_boot.h"

#include <Arduino.h>

void serialBootBegin() {
  Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
  // Sem host USB, write/flush do CDC pode atrasar ou travar o ESP32-C3.
  Serial.setTxTimeoutMs(0);
#endif
}

void serialBootFlush() {
#if ARDUINO_USB_CDC_ON_BOOT
  // Serial.flush() com CDC e sem host e um hang conhecido no Super Mini.
  // setTxTimeoutMs(0) ja evita bloqueio nos writes; nao forcar flush.
  (void)0;
#else
  Serial.flush();
#endif
}
