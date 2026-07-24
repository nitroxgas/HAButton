#pragma once

#include <stdint.h>

struct ButtonState {
  bool a;
  bool b;
  bool fromGpioWake;
};

// Configura pinos e le o estado apos o boot (inclui janela para "ambos").
ButtonState buttonsReadOnBoot();

// Prepara wake por GPIO (nivel baixo) e entra em deep sleep.
void enterDeepSleep();
