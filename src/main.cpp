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

void effectPulseOn() {
  effectOutRampToTarget();
  g_effectOn = true;
  g_effectAtTargetMs = millis();
}

void effectPulseOff(const AppConfig& cfg) {
  if (!g_effectOn) {
    return;
  }
  const uint32_t holdMin = cfg.effectHoldMsValue();
  const uint32_t elapsed = millis() - g_effectAtTargetMs;
  if (elapsed < holdMin) {
    delay(holdMin - elapsed);
  }
  effectOutOff();
  g_effectOn = false;
}

void effectForceOff() {
  effectOutOff();
  g_effectOn = false;
}

// Fluxo: GPIO7 ON -> publish MQTT -> hold min -> GPIO7 OFF.
// Com mirror LED→effect ativo, o GPIO7 só segue o LED azul.
void publishWithEffect(AppConfig& cfg, const char* eventType) {
  if (eventType == nullptr || eventType[0] == '\0') {
    return;
  }
  const bool mirror = cfg.effectMirrorLedEnabled();
  if (!mirror) {
    effectPulseOn();
  }
  mqttPublishGesture(cfg, eventType);
  if (!mirror) {
    effectPulseOff(cfg);
  }
}

void logBootPinStates() {
  // Não remapeia GPIO8 (LED): só lê sem mudar pinMode do status LED.
  const int pins[] = {0, 2, 3, 4, 5, 9};
  Serial.print("[boot] niveis GPIO ");
  for (int pin : pins) {
    pinMode(pin, INPUT);
    Serial.printf("%d=%d ", pin, digitalRead(pin));
  }
  Serial.printf("8(led)=%d ", digitalRead(STATUS_LED_PIN));
  Serial.println();
}

}  // namespace

void setup() {
  serialBootBegin();
  logBootPinStates();

  statusLedBegin();
  statusLedOn();

  effectOutBegin();
  buttonsBegin();

  const bool wokeByGpio =
      (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO);

  // Identifica o botao do wake ANTES do Wi-Fi (GPIO7 ainda off).
  const char* wakeEvent = nullptr;
  if (wokeByGpio || buttonsAnyDown()) {
    wakeEvent = buttonsCaptureWakePress();
    Serial.printf("[main] wake mask/event=%s\n",
                  wakeEvent != nullptr ? wakeEvent : "(nenhum)");
  }

  delay(30);
  // LED permanece aceso na inicialização até o Wi‑Fi assumir o modo (pisca/portal).
  Serial.println();
  Serial.printf("=== HAButton ESP32-C3 %s ===\n", FW_VERSION);
  Serial.printf("[main] wake=%s btn_down=%d\n", wokeByGpio ? "GPIO" : "outro",
                buttonsAnyDown() ? 1 : 0);

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
  } else {
    char detail[96];
    snprintf(detail, sizeof(detail), "wake=%s event=%s ip=%s",
             wokeByGpio ? "gpio" : "other",
             wakeEvent != nullptr ? wakeEvent : "none",
             WiFi.localIP().toString().c_str());
    mqttPublishLog(cfg, "session_start", detail);
  }

  // Publica o gesto que acordou: ON GPIO7 -> MQTT -> OFF GPIO7.
  if (wakeEvent != nullptr) {
    publishWithEffect(cfg, wakeEvent);
    buttonsWaitReleaseAndReset();
  } else {
    buttonsWaitReleaseAndReset();
  }

  // Sessão idle: LED pisca lento; pulsos rápidos em publish MQTT.
  statusLedSetMode(StatusLedMode::BlinkIdle);
  uint32_t idleDeadline = millis() + cfg.sleepDelayMsValue();
  Serial.printf("[main] sessao acordada idle=%u ms debug=%d\n",
                cfg.sleepDelayMsValue(), cfg.debugMqttEnabled() ? 1 : 0);

  bool sleepWarned = false;

  while (millis() < idleDeadline) {
    statusLedTick();
    otaHandle();
    mqttHandle(cfg);

    if (otaIsInProgress()) {
      idleDeadline = millis() + cfg.sleepDelayMsValue();
      sleepWarned = false;
      delay(5);
      continue;
    }

    mqttEnsureConnected(cfg);

    const uint32_t remain = (idleDeadline > millis()) ? (idleDeadline - millis()) : 0;
    if (!sleepWarned && remain > 0 && remain <= 2000) {
      sleepWarned = true;
      mqttPublishLog(cfg, "sleep_soon", "idle_expiring");
      Serial.println("[main] idle quase esgotado — sleep em breve");
    }

    const GestureResult g = buttonsPollGesture();

    if (g.isConfigChord) {
      Serial.println("[main] config chord — reiniciando no portal");
      mqttPublishLog(cfg, "portal_request", "abc_10s");
      wifiRequestConfigPortalOnNextBoot();
      effectForceOff();
      statusLedOff();
      delay(100);
      ESP.restart();
    }

    if (g.isRediscoverChord) {
      Serial.println("[main] A+C 10s — republicando discovery/config MQTT");
      statusLedOn();
      const bool ok = mqttRepublishAll(cfg);
      Serial.printf("[main] rediscover -> %s\n", ok ? "ok" : "FAIL");
      buttonsWaitReleaseAndReset();
      statusLedSetMode(StatusLedMode::BlinkIdle);
      idleDeadline = millis() + cfg.sleepDelayMsValue();
      sleepWarned = false;
      continue;
    }

    // Proximos acionamentos na sessao: identifica no release, depois GPIO7+MQTT+OFF.
    if (g.hasEvent && g.eventType != nullptr) {
      publishWithEffect(cfg, g.eventType);
      idleDeadline = millis() + cfg.sleepDelayMsValue();
      sleepWarned = false;
      Serial.printf("[main] idle reset -> %u ms\n", cfg.sleepDelayMsValue());
    }

    delay(5);
  }

  Serial.println("[main] idle esgotado — deep sleep");
  mqttPublishLog(cfg, "sleep_enter", "idle_timeout");
  delay(80);
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
