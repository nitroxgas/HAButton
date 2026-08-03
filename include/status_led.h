#pragma once

#include <stdint.h>

enum class StatusLedMode : uint8_t {
  Off = 0,
  On,
  BlinkWifi,  // período 1 s (500 ms ligado / 500 ms apagado)
  BlinkIdle,  // período 2 s (pisca lento na sessão idle)
};

void statusLedBegin();
void statusLedSetMode(StatusLedMode mode);
void statusLedOn();   // equivalente a StatusLedMode::On
void statusLedOff();  // equivalente a StatusLedMode::Off
// Pisca rápido (override breve) ao publicar MQTT; depois restaura o modo atual.
void statusLedPulseMqtt();
void statusLedTick();  // opcional; o ticker interno já chama
