#pragma once

#include <Arduino.h>

void effectOutBegin();
void effectOutSetTargetMv(uint16_t mv);
void effectOutRampToTarget();
void effectOutOff();

// Quando ativo, o GPIO7 segue o LED azul (status) para conferência visual.
void effectOutSetMirrorLed(bool enable);
bool effectOutMirrorLedEnabled();
void effectOutMirrorFromLed(bool ledOn);
