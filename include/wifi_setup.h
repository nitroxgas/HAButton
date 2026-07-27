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
  String debugMqtt;      // "0" / "1"
  String longPressMs;
  String effectHoldMs;
  String effectTargetMv;

  uint32_t sleepDelayMsValue() const;
  bool debugMqttEnabled() const;
  uint32_t longPressMsValue() const;
  uint32_t effectHoldMsValue() const;
  uint16_t effectTargetMvValue() const;
};

bool wifiSetupAndConnect(AppConfig& cfg);
void wifiLoadConfig(AppConfig& cfg);
void wifiSaveConfig(const AppConfig& cfg);
void wifiRequestConfigPortalOnNextBoot();

// Aplica overrides de timing/efeito apos carregar config (botoes + PWM).
void wifiApplyRuntimeTuning(const AppConfig& cfg);
