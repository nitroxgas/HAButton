#include "wifi_setup.h"
#include "config.h"
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
}

void loadFromNvs(AppConfig& cfg) {
  applyDefaults(cfg);
  if (!prefs.begin(NVS_NAMESPACE, true)) {
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
  prefs.end();
}

bool configChanged(const AppConfig& before, const AppConfig& after) {
  return before.mqttHost != after.mqttHost || before.mqttPort != after.mqttPort ||
         before.mqttUser != after.mqttUser || before.mqttPass != after.mqttPass ||
         before.mqttPrefix != after.mqttPrefix || before.deviceName != after.deviceName ||
         before.btnAName != after.btnAName || before.btnBName != after.btnBName ||
         before.btnCName != after.btnCName || before.sleepDelayMs != after.sleepDelayMs ||
         before.otaPass != after.otaPass;
}

void saveToNvs(const AppConfig& cfg) {
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    Serial.println("[wifi] falha ao abrir NVS para gravacao");
    return;
  }
  prefs.putString("mqtt_host", cfg.mqttHost);
  prefs.putString("mqtt_port", cfg.mqttPort);
  prefs.putString("mqtt_user", cfg.mqttUser);
  prefs.putString("mqtt_pass", cfg.mqttPass);
  String prefix = cfg.mqttPrefix;
  prefix.trim();
  if (prefix.isEmpty()) {
    prefix = DEFAULT_MQTT_PREFIX;
  }
  prefs.putString("mqtt_prefix", prefix);
  prefs.putString("device_name", cfg.deviceName);
  prefs.putString("btn_a_name", cfg.btnAName);
  prefs.putString("btn_b_name", cfg.btnBName);
  prefs.putString("btn_c_name", cfg.btnCName);
  prefs.putString("sleep_delay", cfg.sleepDelayMs);
  prefs.putString("ota_pass", cfg.otaPass);
  // Republicar discovery HA apos mudanca de config MQTT/device.
  prefs.putBool("disc_done", false);
  prefs.putUChar("disc_ver", 0);
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

void wifiLoadConfig(AppConfig& cfg) {
  loadFromNvs(cfg);
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

  strncpy(mqttHost, cfg.mqttHost.c_str(), sizeof(mqttHost) - 1);
  mqttHost[sizeof(mqttHost) - 1] = '\0';
  strncpy(mqttPort, cfg.mqttPort.c_str(), sizeof(mqttPort) - 1);
  mqttPort[sizeof(mqttPort) - 1] = '\0';
  strncpy(mqttUser, cfg.mqttUser.c_str(), sizeof(mqttUser) - 1);
  mqttUser[sizeof(mqttUser) - 1] = '\0';
  strncpy(mqttPass, cfg.mqttPass.c_str(), sizeof(mqttPass) - 1);
  mqttPass[sizeof(mqttPass) - 1] = '\0';
  strncpy(mqttPrefix, cfg.mqttPrefix.c_str(), sizeof(mqttPrefix) - 1);
  mqttPrefix[sizeof(mqttPrefix) - 1] = '\0';
  strncpy(deviceName, cfg.deviceName.c_str(), sizeof(deviceName) - 1);
  deviceName[sizeof(deviceName) - 1] = '\0';
  strncpy(btnAName, cfg.btnAName.c_str(), sizeof(btnAName) - 1);
  btnAName[sizeof(btnAName) - 1] = '\0';
  strncpy(btnBName, cfg.btnBName.c_str(), sizeof(btnBName) - 1);
  btnBName[sizeof(btnBName) - 1] = '\0';
  strncpy(btnCName, cfg.btnCName.c_str(), sizeof(btnCName) - 1);
  btnCName[sizeof(btnCName) - 1] = '\0';
  strncpy(sleepDelay, cfg.sleepDelayMs.c_str(), sizeof(sleepDelay) - 1);
  sleepDelay[sizeof(sleepDelay) - 1] = '\0';
  strncpy(otaPass, cfg.otaPass.c_str(), sizeof(otaPass) - 1);
  otaPass[sizeof(otaPass) - 1] = '\0';

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

  const String apName = makeApName();
  bool ok = false;

  if (forcePortal) {
    statusLedOn();
    Serial.printf("[wifi] portal forçado AP=%s (timeout %ds)\n",
                  apName.c_str(), WIFI_PORTAL_FORCED_TIMEOUT_S);
    ok = wm.startConfigPortal(apName.c_str());
  } else {
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

  if (forcePortal || configChanged(cfgBefore, cfg)) {
    saveToNvs(cfg);
  }

  if (!ok) {
    Serial.println("[wifi] falha ao conectar / portal expirou");
    statusLedOff();
    return false;
  }

  Serial.printf("[wifi] conectado SSID=%s IP=%s sleep_delay=%u\n",
                WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str(),
                cfg.sleepDelayMsValue());
  return true;
}
