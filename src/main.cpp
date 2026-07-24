#include <Arduino.h>
#include <WiFi.h>

#include "buttons.h"
#include "config.h"
#include "mqtt_ha.h"
#include "status_led.h"
#include "wifi_setup.h"

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== HAButton ESP32-C3 ===");

  statusLedBegin();

  const ButtonState buttons = buttonsReadOnBoot();

  AppConfig cfg;
  const bool wifiOk = wifiSetupAndConnect(cfg);
  if (!wifiOk) {
    Serial.println("[main] Wi-Fi indisponivel; indo dormir");
    statusLedOff();
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
    enterDeepSleep();
    return;
  }

  // LED aceso enquanto conectado e publicando MQTT.
  statusLedOn();
  mqttPublishButtonEvent(cfg, buttons);

  statusLedOff();
  WiFi.disconnect(true, false);
  WiFi.mode(WIFI_OFF);
  enterDeepSleep();
}

void loop() {
  // Nunca alcançado — deep sleep no setup.
}
