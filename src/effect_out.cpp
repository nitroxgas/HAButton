#include "effect_out.h"
#include "config.h"

#include <Arduino.h>

namespace {

uint16_t g_targetMv = EFFECT_TARGET_MV;
bool g_mirrorLed = false;

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
  g_mirrorLed = false;
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
  // Em mirror, o LED status comanda o GPIO7.
  if (g_mirrorLed) {
    return;
  }
  ledcWrite(EFFECT_PWM_CHANNEL, targetDuty());
}

void effectOutOff() {
  if (g_mirrorLed) {
    return;
  }
  ledcWrite(EFFECT_PWM_CHANNEL, 0);
}

void effectOutSetMirrorLed(bool enable) {
  g_mirrorLed = enable;
  if (!enable) {
    ledcWrite(EFFECT_PWM_CHANNEL, 0);
  }
  Serial.printf("[effect] mirror_led=%d\n", enable ? 1 : 0);
}

bool effectOutMirrorLedEnabled() {
  return g_mirrorLed;
}

void effectOutMirrorFromLed(bool ledOn) {
  if (!g_mirrorLed) {
    return;
  }
  ledcWrite(EFFECT_PWM_CHANNEL, ledOn ? targetDuty() : 0);
}
