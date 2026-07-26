#pragma once

#include "wifi_setup.h"

bool mqttEnsureConnected(const AppConfig& cfg);
bool mqttPublishGesture(const AppConfig& cfg, const char* eventType);
void mqttDisconnect();
