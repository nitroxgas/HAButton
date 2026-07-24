#include "buttons.h"
#include "config.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp_sleep.h>

ButtonState buttonsReadOnBoot() {
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  pinMode(BTN_B_PIN, INPUT_PULLUP);

  ButtonState state{};
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  state.fromGpioWake = (cause == ESP_SLEEP_WAKEUP_GPIO);

  // Leitura imediata
  state.a = digitalRead(BTN_A_PIN) == LOW;
  state.b = digitalRead(BTN_B_PIN) == LOW;

  // Janela curta para capturar o segundo botao em acionamento proximo
  const uint32_t deadline = millis() + BUTTON_BOTH_WINDOW_MS;
  while (millis() < deadline) {
    if (digitalRead(BTN_A_PIN) == LOW) {
      state.a = true;
    }
    if (digitalRead(BTN_B_PIN) == LOW) {
      state.b = true;
    }
    if (state.a && state.b) {
      break;
    }
    delay(5);
  }

  Serial.printf("[buttons] wake=%s A=%d B=%d\n",
                state.fromGpioWake ? "gpio" : "other",
                state.a ? 1 : 0,
                state.b ? 1 : 0);
  return state;
}

void enterDeepSleep() {
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  pinMode(BTN_B_PIN, INPUT_PULLUP);

  // Garante que nao dormimos enquanto o botao ainda esta pressionado
  // (evita wake imediato em loop). Espera soltar ou timeout curto.
  const uint32_t releaseDeadline = millis() + 3000;
  while (millis() < releaseDeadline) {
    const bool aDown = digitalRead(BTN_A_PIN) == LOW;
    const bool bDown = digitalRead(BTN_B_PIN) == LOW;
    if (!aDown && !bDown) {
      break;
    }
    delay(20);
  }
  // Debounce apos soltar
  delay(30);

  uint64_t mask = 0;
  mask |= (1ULL << BTN_A_PIN);
  mask |= (1ULL << BTN_B_PIN);

  esp_deep_sleep_enable_gpio_wakeup(mask, ESP_GPIO_WAKEUP_GPIO_LOW);

  Serial.println("[sleep] entrando em deep sleep...");
  Serial.flush();
  delay(50);
  esp_deep_sleep_start();
}
