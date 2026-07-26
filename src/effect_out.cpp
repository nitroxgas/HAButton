#include "effect_out.h"
#include "config.h"

#include <Arduino.h>

namespace {

uint32_t targetDuty() {
  const uint32_t maxDuty = (1u << EFFECT_PWM_RES_BITS) - 1u;
  return (maxDuty * static_cast<uint32_t>(EFFECT_TARGET_MV)) /
         static_cast<uint32_t>(EFFECT_SUPPLY_MV);
}

}  // namespace

void effectOutBegin() {
  ledcSetup(EFFECT_PWM_CHANNEL, EFFECT_PWM_FREQ_HZ, EFFECT_PWM_RES_BITS);
  ledcAttachPin(EFFECT_PIN, EFFECT_PWM_CHANNEL);
  ledcWrite(EFFECT_PWM_CHANNEL, 0);
}

void effectOutRampToTarget() {
  // Chaveamento imediato LOW -> nivel alvo (~2,5 V PWM), sem rampa.
  ledcWrite(EFFECT_PWM_CHANNEL, targetDuty());
}

void effectOutOff() {
  ledcWrite(EFFECT_PWM_CHANNEL, 0);
}
