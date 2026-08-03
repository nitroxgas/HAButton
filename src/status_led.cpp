#include "status_led.h"
#include "config.h"
#include "effect_out.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {

constexpr uint32_t kWifiHalfPeriodMs = 500;   // 1 s ciclo
constexpr uint32_t kIdleHalfPeriodMs = 800;   // ~1,6 s ciclo (visível, ainda mais lento que Wi‑Fi)
constexpr uint32_t kMqttPulseOnMs = 40;
constexpr uint32_t kMqttPulseOffMs = 40;
constexpr uint8_t kMqttPulseCount = 2;
constexpr uint32_t kTickMs = 40;

StatusLedMode g_mode = StatusLedMode::Off;
bool g_levelOn = false;
uint32_t g_phaseStartMs = 0;

bool g_pulseActive = false;
uint8_t g_pulsePhase = 0;
uint8_t g_pulseLeft = 0;
uint32_t g_pulsePhaseStartMs = 0;

TaskHandle_t g_task = nullptr;

inline void writeLed(bool on) {
  // Reafirma OUTPUT: logBootPinStates / libs podem alterar o pino.
  pinMode(STATUS_LED_PIN, OUTPUT);
#if STATUS_LED_ACTIVE_LOW
  digitalWrite(STATUS_LED_PIN, on ? LOW : HIGH);
#else
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
#endif
  g_levelOn = on;
  effectOutMirrorFromLed(on);
}

bool isBlinkMode(StatusLedMode mode) {
  return mode == StatusLedMode::BlinkWifi || mode == StatusLedMode::BlinkIdle;
}

uint32_t blinkHalfMs(StatusLedMode mode) {
  return mode == StatusLedMode::BlinkIdle ? kIdleHalfPeriodMs : kWifiHalfPeriodMs;
}

const char* modeName(StatusLedMode mode) {
  switch (mode) {
    case StatusLedMode::Off:
      return "off";
    case StatusLedMode::On:
      return "on";
    case StatusLedMode::BlinkWifi:
      return "blink_wifi";
    case StatusLedMode::BlinkIdle:
      return "blink_idle";
    default:
      return "?";
  }
}

void applySteady() {
  switch (g_mode) {
    case StatusLedMode::Off:
      writeLed(false);
      break;
    case StatusLedMode::On:
      writeLed(true);
      break;
    case StatusLedMode::BlinkWifi:
    case StatusLedMode::BlinkIdle:
      writeLed(true);
      g_phaseStartMs = millis();
      break;
  }
}

void tickImpl() {
  const uint32_t now = millis();

  if (g_pulseActive) {
    const uint32_t dur = (g_pulsePhase == 0) ? kMqttPulseOnMs : kMqttPulseOffMs;
    if (now - g_pulsePhaseStartMs >= dur) {
      g_pulsePhaseStartMs = now;
      if (g_pulsePhase == 0) {
        g_pulsePhase = 1;
        writeLed(false);
      } else {
        g_pulseLeft--;
        if (g_pulseLeft == 0) {
          g_pulseActive = false;
          applySteady();
          if (isBlinkMode(g_mode)) {
            g_phaseStartMs = now;
          }
          return;
        }
        g_pulsePhase = 0;
        writeLed(true);
      }
    }
    return;
  }

  if (isBlinkMode(g_mode)) {
    if (now - g_phaseStartMs >= blinkHalfMs(g_mode)) {
      g_phaseStartMs = now;
      writeLed(!g_levelOn);
    }
  }
}

void ledTask(void* /*arg*/) {
  for (;;) {
    tickImpl();
    vTaskDelay(pdMS_TO_TICKS(kTickMs));
  }
}

}  // namespace

void statusLedBegin() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  g_mode = StatusLedMode::Off;
  g_pulseActive = false;
  writeLed(false);

  if (g_task == nullptr) {
    // Prioridade um pouco acima do idle; stack pequena.
    xTaskCreatePinnedToCore(ledTask, "status_led", 2048, nullptr, 1, &g_task, 0);
  }

  Serial.printf("[led] begin pin=%d active_low=%d\n", STATUS_LED_PIN,
                STATUS_LED_ACTIVE_LOW ? 1 : 0);
}

void statusLedSetMode(StatusLedMode mode) {
  if (g_mode != mode) {
    Serial.printf("[led] mode=%s\n", modeName(mode));
  }
  g_mode = mode;
  if (!g_pulseActive) {
    applySteady();
  }
}

void statusLedOn() {
  statusLedSetMode(StatusLedMode::On);
}

void statusLedOff() {
  statusLedSetMode(StatusLedMode::Off);
}

void statusLedPulseMqtt() {
  g_pulseActive = true;
  g_pulseLeft = kMqttPulseCount;
  g_pulsePhase = 0;
  g_pulsePhaseStartMs = millis();
  writeLed(true);
}

void statusLedTick() {
  tickImpl();
}
