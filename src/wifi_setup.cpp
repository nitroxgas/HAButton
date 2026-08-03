#include "wifi_setup.h"
#include "buttons.h"
#include "config.h"
#include "effect_out.h"
#include "status_led.h"

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiManager.h>

namespace {

Preferences prefs;
constexpr const char* kForcePortalKey = "force_portal";

void applyDefaults(AppConfig& cfg) {
  cfg.mqttHost = DEFAULT_MQTT_HOST;
  cfg.mqttPort = DEFAULT_MQTT_PORT;
  cfg.mqttUser = DEFAULT_MQTT_USER;
  cfg.mqttPass = DEFAULT_MQTT_PASS;
  cfg.mqttPrefix = DEFAULT_MQTT_PREFIX;
  cfg.deviceName = DEFAULT_DEVICE_NAME;
  cfg.btnAName = DEFAULT_BTN_A_NAME;
  cfg.btnBName = DEFAULT_BTN_B_NAME;
  cfg.btnCName = DEFAULT_BTN_C_NAME;
  cfg.sleepDelayMs = DEFAULT_SLEEP_DELAY_MS;
  cfg.otaPass = DEFAULT_OTA_PASS;
  cfg.debugMqtt = DEFAULT_DEBUG_MQTT;
  cfg.longPressMs = DEFAULT_LONG_PRESS_MS;
  cfg.effectHoldMs = DEFAULT_EFFECT_HOLD_MS;
  cfg.effectTargetMv = DEFAULT_EFFECT_TARGET_MV;
  cfg.effectMirrorLed = DEFAULT_EFFECT_MIRROR_LED;
}

void normalizeConfig(AppConfig& cfg) {
  cfg.mqttPrefix.trim();
  if (cfg.mqttPrefix.isEmpty()) {
    cfg.mqttPrefix = DEFAULT_MQTT_PREFIX;
  }
  cfg.sleepDelayMs.trim();
  if (cfg.sleepDelayMs.isEmpty()) {
    cfg.sleepDelayMs = DEFAULT_SLEEP_DELAY_MS;
  }
  if (cfg.otaPass.isEmpty()) {
    cfg.otaPass = DEFAULT_OTA_PASS;
  }
  cfg.debugMqtt.trim();
  if (cfg.debugMqtt != "1" && cfg.debugMqtt != "true" && cfg.debugMqtt != "ON") {
    cfg.debugMqtt = "0";
  } else {
    cfg.debugMqtt = "1";
  }
  cfg.longPressMs.trim();
  if (cfg.longPressMs.isEmpty()) {
    cfg.longPressMs = DEFAULT_LONG_PRESS_MS;
  }
  cfg.effectHoldMs.trim();
  if (cfg.effectHoldMs.isEmpty()) {
    cfg.effectHoldMs = DEFAULT_EFFECT_HOLD_MS;
  }
  cfg.effectTargetMv.trim();
  if (cfg.effectTargetMv.isEmpty()) {
    cfg.effectTargetMv = DEFAULT_EFFECT_TARGET_MV;
  }
  cfg.effectMirrorLed.trim();
  if (cfg.effectMirrorLed != "1" && cfg.effectMirrorLed != "true" &&
      cfg.effectMirrorLed != "ON") {
    cfg.effectMirrorLed = "0";
  } else {
    cfg.effectMirrorLed = "1";
  }
}

void loadFromNvs(AppConfig& cfg) {
  applyDefaults(cfg);
  if (!prefs.begin(NVS_NAMESPACE, true)) {
    normalizeConfig(cfg);
    return;
  }
  cfg.mqttHost = prefs.getString("mqtt_host", cfg.mqttHost);
  cfg.mqttPort = prefs.getString("mqtt_port", cfg.mqttPort);
  cfg.mqttUser = prefs.getString("mqtt_user", cfg.mqttUser);
  cfg.mqttPass = prefs.getString("mqtt_pass", cfg.mqttPass);
  cfg.mqttPrefix = prefs.getString("mqtt_prefix", cfg.mqttPrefix);
  cfg.deviceName = prefs.getString("device_name", cfg.deviceName);
  cfg.btnAName = prefs.getString("btn_a_name", cfg.btnAName);
  cfg.btnBName = prefs.getString("btn_b_name", cfg.btnBName);
  cfg.btnCName = prefs.getString("btn_c_name", cfg.btnCName);
  cfg.sleepDelayMs = prefs.getString("sleep_delay", cfg.sleepDelayMs);
  cfg.otaPass = prefs.getString("ota_pass", cfg.otaPass);
  cfg.debugMqtt = prefs.getString("debug_mqtt", cfg.debugMqtt);
  cfg.longPressMs = prefs.getString("long_press", cfg.longPressMs);
  cfg.effectHoldMs = prefs.getString("effect_hold", cfg.effectHoldMs);
  cfg.effectTargetMv = prefs.getString("effect_mv", cfg.effectTargetMv);
  cfg.effectMirrorLed = prefs.getString("effect_mirror", cfg.effectMirrorLed);
  prefs.end();
  normalizeConfig(cfg);
}

bool configChanged(const AppConfig& before, const AppConfig& after) {
  return before.mqttHost != after.mqttHost || before.mqttPort != after.mqttPort ||
         before.mqttUser != after.mqttUser || before.mqttPass != after.mqttPass ||
         before.mqttPrefix != after.mqttPrefix || before.deviceName != after.deviceName ||
         before.btnAName != after.btnAName || before.btnBName != after.btnBName ||
         before.btnCName != after.btnCName || before.sleepDelayMs != after.sleepDelayMs ||
         before.otaPass != after.otaPass || before.debugMqtt != after.debugMqtt ||
         before.longPressMs != after.longPressMs || before.effectHoldMs != after.effectHoldMs ||
         before.effectTargetMv != after.effectTargetMv ||
         before.effectMirrorLed != after.effectMirrorLed;
}

void saveToNvs(const AppConfig& cfgIn, bool bumpDiscovery) {
  AppConfig cfg = cfgIn;
  normalizeConfig(cfg);
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    Serial.println("[wifi] falha ao abrir NVS para gravacao");
    return;
  }
  prefs.putString("mqtt_host", cfg.mqttHost);
  prefs.putString("mqtt_port", cfg.mqttPort);
  prefs.putString("mqtt_user", cfg.mqttUser);
  prefs.putString("mqtt_pass", cfg.mqttPass);
  prefs.putString("mqtt_prefix", cfg.mqttPrefix);
  prefs.putString("device_name", cfg.deviceName);
  prefs.putString("btn_a_name", cfg.btnAName);
  prefs.putString("btn_b_name", cfg.btnBName);
  prefs.putString("btn_c_name", cfg.btnCName);
  prefs.putString("sleep_delay", cfg.sleepDelayMs);
  prefs.putString("ota_pass", cfg.otaPass);
  prefs.putString("debug_mqtt", cfg.debugMqtt);
  prefs.putString("long_press", cfg.longPressMs);
  prefs.putString("effect_hold", cfg.effectHoldMs);
  prefs.putString("effect_mv", cfg.effectTargetMv);
  prefs.putString("effect_mirror", cfg.effectMirrorLed);
  if (bumpDiscovery) {
    prefs.putBool("disc_done", false);
    prefs.putUChar("disc_ver", 0);
  }
  prefs.end();
}

String makeApName() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[32];
  snprintf(buf, sizeof(buf), "%s-%02X%02X", WM_AP_NAME_PREFIX, mac[4], mac[5]);
  return String(buf);
}

bool consumeForcePortalFlag() {
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    return false;
  }
  const bool forced = prefs.getBool(kForcePortalKey, false);
  if (forced) {
    prefs.putBool(kForcePortalKey, false);
  }
  prefs.end();
  return forced;
}

}  // namespace

uint32_t AppConfig::sleepDelayMsValue() const {
  long v = sleepDelayMs.toInt();
  if (v < SLEEP_DELAY_MIN_MS) {
    v = SLEEP_DELAY_MIN_MS;
  }
  if (v > SLEEP_DELAY_MAX_MS) {
    v = SLEEP_DELAY_MAX_MS;
  }
  return static_cast<uint32_t>(v);
}

bool AppConfig::debugMqttEnabled() const {
  return debugMqtt == "1";
}

uint32_t AppConfig::longPressMsValue() const {
  long v = longPressMs.toInt();
  if (v < 200) {
    v = 200;
  }
  if (v > 10000) {
    v = 10000;
  }
  return static_cast<uint32_t>(v);
}

uint32_t AppConfig::effectHoldMsValue() const {
  long v = effectHoldMs.toInt();
  if (v < 0) {
    v = 0;
  }
  if (v > 10000) {
    v = 10000;
  }
  return static_cast<uint32_t>(v);
}

uint16_t AppConfig::effectTargetMvValue() const {
  long v = effectTargetMv.toInt();
  if (v < 100) {
    v = 100;
  }
  if (v > EFFECT_SUPPLY_MV) {
    v = EFFECT_SUPPLY_MV;
  }
  return static_cast<uint16_t>(v);
}

bool AppConfig::effectMirrorLedEnabled() const {
  return effectMirrorLed == "1";
}

void wifiLoadConfig(AppConfig& cfg) {
  loadFromNvs(cfg);
}

void wifiSaveConfig(const AppConfig& cfg) {
  saveToNvs(cfg, false);
}

void wifiApplyRuntimeTuning(const AppConfig& cfg) {
  buttonsSetLongPressMs(cfg.longPressMsValue());
  effectOutSetTargetMv(cfg.effectTargetMvValue());
  effectOutSetMirrorLed(cfg.effectMirrorLedEnabled());
}

void wifiRequestConfigPortalOnNextBoot() {
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    Serial.println("[wifi] falha ao gravar flag force_portal");
    return;
  }
  prefs.putBool(kForcePortalKey, true);
  prefs.end();
  Serial.println("[wifi] flag force_portal gravada");
}

bool wifiSetupAndConnect(AppConfig& cfg) {
  loadFromNvs(cfg);
  const AppConfig cfgBefore = cfg;

  const bool forcePortal = consumeForcePortalFlag();
  if (forcePortal) {
    Serial.println("[wifi] boot com force_portal — abrindo portal");
  }

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setDebugOutput(true);
  wm.setConfigPortalTimeout(forcePortal ? WIFI_PORTAL_FORCED_TIMEOUT_S
                                        : WIFI_PORTAL_TIMEOUT_S);
  wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_S);
  wm.setBreakAfterConfig(true);

  char mqttHost[64];
  char mqttPort[8];
  char mqttUser[40];
  char mqttPass[64];
  char mqttPrefix[40];
  char deviceName[40];
  char btnAName[40];
  char btnBName[40];
  char btnCName[40];
  char sleepDelay[12];
  char otaPass[40];
  char debugMqtt[4];
  char longPress[8];
  char effectHold[8];
  char effectMv[8];
  char effectMirror[4];

  auto copyField = [](char* dst, size_t n, const String& src) {
    strncpy(dst, src.c_str(), n - 1);
    dst[n - 1] = '\0';
  };

  copyField(mqttHost, sizeof(mqttHost), cfg.mqttHost);
  copyField(mqttPort, sizeof(mqttPort), cfg.mqttPort);
  copyField(mqttUser, sizeof(mqttUser), cfg.mqttUser);
  copyField(mqttPass, sizeof(mqttPass), cfg.mqttPass);
  copyField(mqttPrefix, sizeof(mqttPrefix), cfg.mqttPrefix);
  copyField(deviceName, sizeof(deviceName), cfg.deviceName);
  copyField(btnAName, sizeof(btnAName), cfg.btnAName);
  copyField(btnBName, sizeof(btnBName), cfg.btnBName);
  copyField(btnCName, sizeof(btnCName), cfg.btnCName);
  copyField(sleepDelay, sizeof(sleepDelay), cfg.sleepDelayMs);
  copyField(otaPass, sizeof(otaPass), cfg.otaPass);
  copyField(debugMqtt, sizeof(debugMqtt), cfg.debugMqtt);
  copyField(longPress, sizeof(longPress), cfg.longPressMs);
  copyField(effectHold, sizeof(effectHold), cfg.effectHoldMs);
  copyField(effectMv, sizeof(effectMv), cfg.effectTargetMv);
  copyField(effectMirror, sizeof(effectMirror), cfg.effectMirrorLed);

  WiFiManagerParameter pHost("mqtt_host", "MQTT Host", mqttHost, sizeof(mqttHost));
  WiFiManagerParameter pPort("mqtt_port", "MQTT Port", mqttPort, sizeof(mqttPort));
  WiFiManagerParameter pUser("mqtt_user", "MQTT User", mqttUser, sizeof(mqttUser));
  WiFiManagerParameter pPass("mqtt_pass", "MQTT Password", mqttPass, sizeof(mqttPass));
  WiFiManagerParameter pPrefix("mqtt_prefix", "HA Discovery Prefix", mqttPrefix, sizeof(mqttPrefix));
  WiFiManagerParameter pDev("device_name", "Device Name", deviceName, sizeof(deviceName));
  WiFiManagerParameter pBtnA("btn_a_name", "Botao A Nome", btnAName, sizeof(btnAName));
  WiFiManagerParameter pBtnB("btn_b_name", "Botao B Nome", btnBName, sizeof(btnBName));
  WiFiManagerParameter pBtnC("btn_c_name", "Botao C Nome", btnCName, sizeof(btnCName));
  WiFiManagerParameter pSleep("sleep_delay", "Sleep delay ms (idle / OTA)", sleepDelay,
                              sizeof(sleepDelay));
  WiFiManagerParameter pOta("ota_pass", "OTA Password", otaPass, sizeof(otaPass));
  WiFiManagerParameter pDebug("debug_mqtt", "Debug MQTT logs (0/1)", debugMqtt, sizeof(debugMqtt));
  WiFiManagerParameter pLong("long_press", "Long press ms", longPress, sizeof(longPress));
  WiFiManagerParameter pHold("effect_hold", "Effect hold ms (min)", effectHold, sizeof(effectHold));
  WiFiManagerParameter pMv("effect_mv", "Effect target mV", effectMv, sizeof(effectMv));
  WiFiManagerParameter pMirror("effect_mirror", "Mirror LED to effect (0/1)", effectMirror,
                               sizeof(effectMirror));

  wm.addParameter(&pHost);
  wm.addParameter(&pPort);
  wm.addParameter(&pUser);
  wm.addParameter(&pPass);
  wm.addParameter(&pPrefix);
  wm.addParameter(&pDev);
  wm.addParameter(&pBtnA);
  wm.addParameter(&pBtnB);
  wm.addParameter(&pBtnC);
  wm.addParameter(&pSleep);
  wm.addParameter(&pOta);
  wm.addParameter(&pDebug);
  wm.addParameter(&pLong);
  wm.addParameter(&pHold);
  wm.addParameter(&pMv);
  wm.addParameter(&pMirror);

  const String apName = makeApName();
  bool ok = false;

  // Portal (AP) → LED fixo; tentativa de STA → pisca 1 s.
  wm.setAPCallback([](WiFiManager*) {
    statusLedSetMode(StatusLedMode::On);
    Serial.println("[wifi] portal ativo — LED fixo");
  });

  if (forcePortal) {
    statusLedSetMode(StatusLedMode::On);
    Serial.printf("[wifi] portal forçado AP=%s (timeout %ds)\n",
                  apName.c_str(), WIFI_PORTAL_FORCED_TIMEOUT_S);
    ok = wm.startConfigPortal(apName.c_str());
  } else {
    statusLedSetMode(StatusLedMode::BlinkWifi);
    Serial.printf("[wifi] tentando conectar / portal AP=%s\n", apName.c_str());
    ok = wm.autoConnect(apName.c_str());
  }

  cfg.mqttHost = pHost.getValue();
  cfg.mqttPort = pPort.getValue();
  cfg.mqttUser = pUser.getValue();
  cfg.mqttPass = pPass.getValue();
  cfg.mqttPrefix = pPrefix.getValue();
  cfg.deviceName = pDev.getValue();
  cfg.btnAName = pBtnA.getValue();
  cfg.btnBName = pBtnB.getValue();
  cfg.btnCName = pBtnC.getValue();
  cfg.sleepDelayMs = pSleep.getValue();
  cfg.otaPass = pOta.getValue();
  cfg.debugMqtt = pDebug.getValue();
  cfg.longPressMs = pLong.getValue();
  cfg.effectHoldMs = pHold.getValue();
  cfg.effectTargetMv = pMv.getValue();
  cfg.effectMirrorLed = pMirror.getValue();
  normalizeConfig(cfg);

  if (forcePortal || configChanged(cfgBefore, cfg)) {
    const bool identityChanged =
        cfgBefore.mqttHost != cfg.mqttHost || cfgBefore.mqttPort != cfg.mqttPort ||
        cfgBefore.mqttUser != cfg.mqttUser || cfgBefore.mqttPass != cfg.mqttPass ||
        cfgBefore.mqttPrefix != cfg.mqttPrefix || cfgBefore.deviceName != cfg.deviceName;
    saveToNvs(cfg, identityChanged || forcePortal);
  }

  wifiApplyRuntimeTuning(cfg);

  if (!ok) {
    Serial.println("[wifi] falha ao conectar / portal expirou");
    statusLedOff();
    return false;
  }

  statusLedOff();
  Serial.printf("[wifi] conectado SSID=%s IP=%s sleep=%u debug=%s\n",
                WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str(),
                cfg.sleepDelayMsValue(),
                cfg.debugMqtt.c_str());
  return true;
}
