#include "ota_update.h"
#include "config.h"
#include "status_led.h"

#include <ArduinoOTA.h>

namespace {

volatile bool g_otaInProgress = false;

}  // namespace

void otaBegin(const String& hostname, const String& password) {
  g_otaInProgress = false;

  ArduinoOTA.setHostname(hostname.c_str());
  if (password.length() > 0) {
    ArduinoOTA.setPassword(password.c_str());
  }

  ArduinoOTA.onStart([]() {
    g_otaInProgress = true;
    Serial.println("[ota] start");
    statusLedOn();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[ota] end");
    g_otaInProgress = false;
    statusLedOff();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    g_otaInProgress = true;
    static unsigned lastPct = 0;
    const unsigned pct = total ? (progress * 100) / total : 0;
    if (pct != lastPct && pct % 10 == 0) {
      Serial.printf("[ota] %u%%\n", pct);
      lastPct = pct;
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ota] error %u\n", static_cast<unsigned>(error));
    g_otaInProgress = false;
    statusLedOff();
  });

  ArduinoOTA.begin();
  Serial.printf("[ota] pronto hostname=%s\n", hostname.c_str());
}

void otaHandle() {
  ArduinoOTA.handle();
}

bool otaIsInProgress() {
  return g_otaInProgress;
}
