#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>

#include "buttons.h"
#include "config.h"
#include "effect_out.h"
#include "mqtt_ha.h"
#include "ota_update.h"
#include "serial_boot.h"
#include "status_led.h"
#include "wifi_setup.h"

namespace {

bool g_effectOn = false;
uint32_t g_effectAtTargetMs = 0;

String otaHostname(const AppConfig& cfg) {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[48];
  snprintf(buf, sizeof(buf), "%s-%02x%02x", cfg.deviceName.c_str(), mac[4], mac[5]);
  return String(buf);
}

void effectStartFromInterrupt() {
  if (g_effectOn) {
    return;
  }
  effectOutRampToTarget();
  g_effectOn = true;
  g_effectAtTargetMs = millis();
}

void effectStopAfterPublish() {
  if (!g_effectOn) {
    effectOutRampToTarget();
    g_effectAtTargetMs = millis();
    g_effectOn = true;
  }
  const uint32_t elapsed = millis() - g_effectAtTargetMs;
  if (elapsed < EFFECT_HOLD_MIN_MS) {
    delay(EFFECT_HOLD_MIN_MS - elapsed);
  }
  effectOutOff();
  g_effectOn = false;
}

void effectForceOff() {
  effectOutOff();
  g_effectOn = false;
}

// Leitura diagnostica (antes de dirigir LED/efeito). GPIO0 NAO e strapping no C3.
void logBootPinStates() {
  const int pins[] = {0, 2, 3, 4, 5, 8, 9};
  Serial.print("[boot] niveis GPIO ");
  for (int pin : pins) {
    pinMode(pin, INPUT);
    Serial.printf("%d=%d ", pin, digitalRead(pin));
  }
  Serial.println();
  Serial.println("[boot] strapping C3: GPIO2/8/9 (nao GPIO0). BOOT=GPIO9 HIGH=app");
  if (digitalRead(9) == LOW) {
    Serial.println("[boot] AVISO: GPIO9 LOW — se assim no reset, entra download mode");
  }
  if (digitalRead(2) == LOW) {
    Serial.println("[boot] AVISO: GPIO2 LOW — risco de boot/flash instavel");
  }
}

}  // namespace

void setup() {
  // Serial ANTES de qualquer log; CDC sem host nao pode bloquear (bateria).
  serialBootBegin();
  logBootPinStates();

  statusLedBegin();
  // Pulso curto: confirma boot na bateria sem monitor USB.
  statusLedOn();

  // Efeito o mais cedo possivel apos wake por GPIO / botao ainda pressionado.
  effectOutBegin();
  buttonsBegin();
  const bool wokeByGpio =
      (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO);
  if (wokeByGpio || buttonsAnyDown()) {
    effectStartFromInterrupt();
    Serial.println("[effect] GPIO7 ativo (interrupcao/press)");
  }

  delay(30);
  statusLedOff();
  Serial.println();
  Serial.printf("=== HAButton ESP32-C3 %s ===\n", FW_VERSION);
  Serial.printf("[main] wake=%s btn_down=%d (A=%d B=%d C=%d)\n",
                wokeByGpio ? "GPIO" : "outro", buttonsAnyDown() ? 1 : 0,
                BTN_A_PIN, BTN_B_PIN, BTN_C_PIN);

  AppConfig cfg;
  const bool wifiOk = wifiSetupAndConnect(cfg);
  if (!wifiOk) {
    Serial.println("[main] Wi-Fi indisponivel; indo dormir");
    statusLedOff();
    effectForceOff();
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
    enterDeepSleep();
    return;
  }

  otaBegin(otaHostname(cfg), cfg.otaPass);

  if (!mqttEnsureConnected(cfg)) {
    Serial.println("[main] MQTT indisponivel; sessao curta sem broker");
  }

  statusLedOn();
  uint32_t idleDeadline = millis() + cfg.sleepDelayMsValue();
  Serial.printf("[main] sessao acordada idle=%u ms\n", cfg.sleepDelayMsValue());

  while (millis() < idleDeadline) {
    otaHandle();

    // Enquanto OTA estiver ativo, nao deixar o idle expirar.
    if (otaIsInProgress()) {
      idleDeadline = millis() + cfg.sleepDelayMsValue();
      delay(5);
      continue;
    }

    mqttEnsureConnected(cfg);

    const GestureResult g = buttonsPollGesture();
    if (g.pressStarted) {
      effectStartFromInterrupt();
    }

    if (g.isConfigChord) {
      Serial.println("[main] config chord — reiniciando no portal");
      wifiRequestConfigPortalOnNextBoot();
      effectForceOff();
      statusLedOff();
      delay(100);
      ESP.restart();
    }

    if (g.hasEvent && g.eventType != nullptr) {
      mqttPublishGesture(cfg, g.eventType);
      effectStopAfterPublish();
      idleDeadline = millis() + cfg.sleepDelayMsValue();
      Serial.printf("[main] idle reset -> %u ms\n", cfg.sleepDelayMsValue());
    }

    delay(5);
  }

  Serial.println("[main] idle esgotado — deep sleep");
  statusLedOff();
  effectForceOff();
  mqttDisconnect();
  WiFi.disconnect(true, false);
  WiFi.mode(WIFI_OFF);
  enterDeepSleep();
}

void loop() {
  // Deep sleep no setup.
}
