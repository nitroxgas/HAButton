#pragma once

#include "wifi_setup.h"

bool mqttEnsureConnected(AppConfig& cfg);
bool mqttPublishGesture(AppConfig& cfg, const char* eventType);
void mqttPublishLog(AppConfig& cfg, const char* event, const char* detail = nullptr);
void mqttHandle(AppConfig& cfg);
void mqttPublishConfigState(const AppConfig& cfg);
void mqttDisconnect();
