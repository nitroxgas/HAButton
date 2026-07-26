#pragma once

struct GestureResult {
  bool hasEvent;
  bool isConfigChord;
  bool pressStarted;  // true no instante em que um botao passa a ser pressionado
  const char* eventType;
};

void buttonsBegin();

// Algum botao A/B/C em nivel baixo (pressionado).
bool buttonsAnyDown();

// Poll nao bloqueante. Completa gesto no release (ou config chord A+B+C).
GestureResult buttonsPollGesture();

void enterDeepSleep();
