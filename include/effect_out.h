#pragma once

#include <Arduino.h>

void effectOutBegin();
void effectOutSetTargetMv(uint16_t mv);
void effectOutRampToTarget();
void effectOutOff();
