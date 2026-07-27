#include "buttons.h"
#include "config.h"
#include "serial_boot.h"
#include "status_led.h"

#include <Arduino.h>
#include <esp_sleep.h>

namespace {

enum class Phase : uint8_t { Idle, Active };

Phase g_phase = Phase::Idle;
uint8_t g_mask = 0;
uint8_t g_latched = 0;
uint32_t g_pressStart = 0;
uint32_t g_chordDeadline = 0;
bool g_longMarked = false;
uint32_t g_longPressMs = LONG_PRESS_MS;

inline bool pinDown(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

uint8_t readMask() {
  uint8_t m = 0;
  if (pinDown(BTN_A_PIN)) {
    m |= 0x01;
  }
  if (pinDown(BTN_B_PIN)) {
    m |= 0x02;
  }
  if (pinDown(BTN_C_PIN)) {
    m |= 0x04;
  }
  return m;
}

const char* mapEvent(uint8_t mask, bool isLong) {
  const char* shortMap[] = {
      nullptr,     "press_a",   "press_b",   "press_ab",
      "press_c",   "press_ac",  "press_bc",  "press_abc",
  };
  const char* longMap[] = {
      nullptr,    "long_a",   "long_b",   "long_ab",
      "long_c",   "long_ac",  "long_bc",  "long_abc",
  };
  if (mask == 0 || mask > 7) {
    return nullptr;
  }
  return isLong ? longMap[mask] : shortMap[mask];
}

}  // namespace

void buttonsBegin() {
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  pinMode(BTN_B_PIN, INPUT_PULLUP);
  pinMode(BTN_C_PIN, INPUT_PULLUP);
  g_phase = Phase::Idle;
  g_mask = 0;
  g_latched = 0;
  g_longMarked = false;
}

bool buttonsAnyDown() {
  return readMask() != 0;
}

uint8_t buttonsReadMask() {
  return readMask();
}

const char* buttonsCaptureWakePress() {
  // Amostra imediata + janela curta para combos (mesmo criterio do poll).
  uint8_t latched = readMask();
  const uint32_t deadline = millis() + BUTTON_CHORD_WINDOW_MS;
  while (millis() < deadline) {
    latched |= readMask();
    if (latched == 0x07) {
      break;
    }
    delay(5);
  }
  // Se ja soltou, ainda assim usamos o que foi latched na janela.
  if (latched == 0) {
    // Ultima tentativa: um botao pode ter acordado e ja solto — releitura unica.
    delay(10);
    latched = readMask();
  }
  return mapEvent(latched, false);
}

void buttonsWaitReleaseAndReset() {
  const uint32_t deadline = millis() + 5000;
  while (millis() < deadline && readMask() != 0) {
    delay(10);
  }
  delay(40);
  g_phase = Phase::Idle;
  g_mask = 0;
  g_latched = 0;
  g_longMarked = false;
}

void buttonsSetLongPressMs(uint32_t ms) {
  if (ms < 200) {
    ms = 200;
  }
  if (ms > 10000) {
    ms = 10000;
  }
  g_longPressMs = ms;
}

GestureResult buttonsPollGesture() {
  GestureResult out{};
  out.hasEvent = false;
  out.isConfigChord = false;
  out.pressStarted = false;
  out.eventType = nullptr;

  const uint8_t now = readMask();

  if (g_phase == Phase::Idle) {
    if (now == 0) {
      return out;
    }
    g_phase = Phase::Active;
    g_pressStart = millis();
    g_chordDeadline = millis() + BUTTON_CHORD_WINDOW_MS;
    g_mask = now;
    g_latched = now;
    g_longMarked = false;
    out.pressStarted = true;
    return out;
  }

  // Active
  if (millis() < g_chordDeadline) {
    g_latched |= now;
  }
  g_mask = now;
  g_latched |= now;

  const uint32_t held = millis() - g_pressStart;

  if (g_latched == 0x07 && held >= CONFIG_CHORD_HOLD_MS) {
    statusLedOn();
    Serial.println("[buttons] A+B+C 10s — portal");
    out.isConfigChord = true;
    g_phase = Phase::Idle;
    return out;
  }

  if (held >= g_longPressMs) {
    g_longMarked = true;
  }

  // Feedback visual em long / config
  if (g_latched == 0x07 && held > g_longPressMs) {
    if ((held / 250) % 2 == 0) {
      statusLedOn();
    } else {
      statusLedOff();
    }
  }

  if (now != 0) {
    return out;
  }

  // Release — emitir gesto
  const char* ev = mapEvent(g_latched, g_longMarked);
  g_phase = Phase::Idle;
  statusLedOff();

  if (ev == nullptr) {
    return out;
  }

  out.hasEvent = true;
  out.eventType = ev;
  Serial.printf("[buttons] gesto=%s latched=0x%02x long=%d\n", ev, g_latched,
                g_longMarked ? 1 : 0);
  return out;
}

void enterDeepSleep() {
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  pinMode(BTN_B_PIN, INPUT_PULLUP);
  pinMode(BTN_C_PIN, INPUT_PULLUP);

  const uint32_t releaseDeadline = millis() + 3000;
  while (millis() < releaseDeadline) {
    if (readMask() == 0) {
      break;
    }
    delay(20);
  }
  delay(30);

  // ESP32-C3: só GPIO0–5 acordam do deep sleep.
  const int wakePins[] = {BTN_A_PIN, BTN_B_PIN, BTN_C_PIN};
  uint64_t mask = 0;
  for (int pin : wakePins) {
    if (esp_sleep_is_valid_wakeup_gpio(static_cast<gpio_num_t>(pin))) {
      mask |= (1ULL << pin);
    } else {
      Serial.printf("[sleep] GPIO%d ignorado (nao e wakeup RTC)\n", pin);
    }
  }
  if (mask == 0) {
    Serial.println("[sleep] ERRO: nenhum pino valido para wake");
  }

  esp_deep_sleep_enable_gpio_wakeup(mask, ESP_GPIO_WAKEUP_GPIO_LOW);

  Serial.println("[sleep] entrando em deep sleep...");
  serialBootFlush();
  delay(20);
  esp_deep_sleep_start();
}
