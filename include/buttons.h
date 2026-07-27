#pragma once

#include <Arduino.h>

struct GestureResult {
  bool hasEvent;
  bool isConfigChord;
  bool pressStarted;  // true no instante em que um botao passa a ser pressionado
  const char* eventType;
};

void buttonsBegin();

// Algum botao A/B/C em nivel baixo (pressionado).
bool buttonsAnyDown();

// Leitura da mascara atual (bit0=A, bit1=B, bit2=C).
uint8_t buttonsReadMask();

// Captura gesto de wake: amostra A/B/C na janela de chord e mapeia press_* (curto).
// Retorna nullptr se nenhum botao.
const char* buttonsCaptureWakePress();

// Espera soltar todos os botoes e zera a maquina de gestos (evita republish do wake).
void buttonsWaitReleaseAndReset();

// Ajusta limiar de long press em runtime (portal / MQTT).
void buttonsSetLongPressMs(uint32_t ms);

// Poll nao bloqueante. Completa gesto no release (ou config chord A+B+C).
GestureResult buttonsPollGesture();

void enterDeepSleep();
