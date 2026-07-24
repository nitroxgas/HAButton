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
};

// Carrega config do NVS (ou defaults) e conecta Wi-Fi / abre portal se preciso.
// Retorna true se Wi-Fi ficou conectado.
bool wifiSetupAndConnect(AppConfig& cfg);

// Apenas carrega a config do NVS sem tentar conectar.
void wifiLoadConfig(AppConfig& cfg);
