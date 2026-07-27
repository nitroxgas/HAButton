#include "effect_out.h"
#include "config.h"

#include <Arduino.h>

namespace {

uint16_t g_targetMv = EFFECT_TARGET_MV;

uint32_t targetDuty() {
  const uint32_t maxDuty = (1u << EFFECT_PWM_RES_BITS) - 1u;
  uint16_t mv = g_targetMv;
  if (mv > EFFECT_SUPPLY_MV) {
    mv = EFFECT_SUPPLY_MV;
  }
  return (maxDuty * static_cast<uint32_t>(mv)) / static_cast<uint32_t>(EFFECT_SUPPLY_MV);
}

}  // namespace

void effectOutBegin() {
  ledcSetup(EFFECT_PWM_CHANNEL, EFFECT_PWM_FREQ_HZ, EFFECT_PWM_RES_BITS);
  ledcAttachPin(EFFECT_PIN, EFFECT_PWM_CHANNEL);
  ledcWrite(EFFECT_PWM_CHANNEL, 0);
  g_targetMv = EFFECT_TARGET_MV;
}

void effectOutSetTargetMv(uint16_t mv) {
  if (mv < 100) {
    mv = 100;
  }
  if (mv > EFFECT_SUPPLY_MV) {
    mv = EFFECT_SUPPLY_MV;
  }
  g_targetMv = mv;
}

void effectOutRampToTarget() {
  // Chaveamento imediato LOW -> nivel alvo (PWM), sem rampa.
  ledcWrite(EFFECT_PWM_CHANNEL, targetDuty());
}

void effectOutOff() {
  ledcWrite(EFFECT_PWM_CHANNEL, 0);
}
