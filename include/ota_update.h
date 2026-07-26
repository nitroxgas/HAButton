#pragma once

#include <Arduino.h>

void otaBegin(const String& hostname, const String& password);
void otaHandle();

// True enquanto um upload OTA estiver em andamento.
bool otaIsInProgress();
