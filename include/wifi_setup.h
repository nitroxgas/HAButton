#pragma once

#include <Arduino.h>

struct AppConfig {
  String mqttHost;
  String mqttPort;
  String mqttUser;
  String mqttPass;
  String mqttPrefix;
  String deviceName;
  String btnAName;
  String btnBName;
  String btnCName;
  String sleepDelayMs;
  String otaPass;

  uint32_t sleepDelayMsValue() const;
};

bool wifiSetupAndConnect(AppConfig& cfg);
void wifiLoadConfig(AppConfig& cfg);
void wifiRequestConfigPortalOnNextBoot();
