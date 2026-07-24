#include <Arduino.h>
#include <WiFi.h>

#include "buttons.h"
#include "config.h"
#include "mqtt_ha.h"
#include "wifi_setup.h"

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== HAButton ESP32-C3 ===");

  const ButtonState buttons = buttonsReadOnBoot();

  AppConfig cfg;
  const bool wifiOk = wifiSetupAndConnect(cfg);
  if (!wifiOk) {
    Serial.println("[main] Wi-Fi indisponivel; indo dormir");
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
    enterDeepSleep();
    return;
  }

  // Em wake por GPIO (ou power-on), publica evento / discovery
  mqttPublishButtonEvent(cfg, buttons);

  WiFi.disconnect(true, false);
  WiFi.mode(WIFI_OFF);
  enterDeepSleep();
}

void loop() {
  // Nunca alcançado — deep sleep no setup.
}
