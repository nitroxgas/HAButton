#include "wifi_setup.h"
#include "config.h"

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiManager.h>

namespace {

Preferences prefs;

void applyDefaults(AppConfig& cfg) {
  cfg.mqttHost = DEFAULT_MQTT_HOST;
  cfg.mqttPort = DEFAULT_MQTT_PORT;
  cfg.mqttUser = DEFAULT_MQTT_USER;
  cfg.mqttPass = DEFAULT_MQTT_PASS;
  cfg.mqttPrefix = DEFAULT_MQTT_PREFIX;
  cfg.deviceName = DEFAULT_DEVICE_NAME;
  cfg.btnAName = DEFAULT_BTN_A_NAME;
  cfg.btnBName = DEFAULT_BTN_B_NAME;
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
  prefs.end();
}

bool configChanged(const AppConfig& before, const AppConfig& after) {
  return before.mqttHost != after.mqttHost || before.mqttPort != after.mqttPort ||
         before.mqttUser != after.mqttUser || before.mqttPass != after.mqttPass ||
         before.mqttPrefix != after.mqttPrefix || before.deviceName != after.deviceName ||
         before.btnAName != after.btnAName || before.btnBName != after.btnBName;
}

void saveToNvs(const AppConfig& cfg, bool resetDiscovery) {
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
  if (resetDiscovery) {
    prefs.putBool("disc_done", false);
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

}  // namespace

void wifiLoadConfig(AppConfig& cfg) {
  loadFromNvs(cfg);
}

bool wifiSetupAndConnect(AppConfig& cfg) {
  loadFromNvs(cfg);
  const AppConfig cfgBefore = cfg;

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setDebugOutput(true);
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_S);
  wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_S);
  wm.setBreakAfterConfig(true);

  // Buffers mutaveis exigidos pelo WiFiManager
  char mqttHost[64];
  char mqttPort[8];
  char mqttUser[40];
  char mqttPass[64];
  char mqttPrefix[40];
  char deviceName[40];
  char btnAName[40];
  char btnBName[40];

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

  WiFiManagerParameter pHost("mqtt_host", "MQTT Host", mqttHost, sizeof(mqttHost));
  WiFiManagerParameter pPort("mqtt_port", "MQTT Port", mqttPort, sizeof(mqttPort));
  WiFiManagerParameter pUser("mqtt_user", "MQTT User", mqttUser, sizeof(mqttUser));
  WiFiManagerParameter pPass("mqtt_pass", "MQTT Password", mqttPass, sizeof(mqttPass));
  WiFiManagerParameter pPrefix("mqtt_prefix", "HA Discovery Prefix", mqttPrefix, sizeof(mqttPrefix));
  WiFiManagerParameter pDev("device_name", "Device Name", deviceName, sizeof(deviceName));
  WiFiManagerParameter pBtnA("btn_a_name", "Botao A Nome", btnAName, sizeof(btnAName));
  WiFiManagerParameter pBtnB("btn_b_name", "Botao B Nome", btnBName, sizeof(btnBName));

  wm.addParameter(&pHost);
  wm.addParameter(&pPort);
  wm.addParameter(&pUser);
  wm.addParameter(&pPass);
  wm.addParameter(&pPrefix);
  wm.addParameter(&pDev);
  wm.addParameter(&pBtnA);
  wm.addParameter(&pBtnB);

  const String apName = makeApName();
  Serial.printf("[wifi] tentando conectar / portal AP=%s\n", apName.c_str());

  // autoConnect: usa credenciais salvas; se falhar, abre portal (timeout)
  const bool ok = wm.autoConnect(apName.c_str());

  cfg.mqttHost = pHost.getValue();
  cfg.mqttPort = pPort.getValue();
  cfg.mqttUser = pUser.getValue();
  cfg.mqttPass = pPass.getValue();
  cfg.mqttPrefix = pPrefix.getValue();
  cfg.deviceName = pDev.getValue();
  cfg.btnAName = pBtnA.getValue();
  cfg.btnBName = pBtnB.getValue();

  if (configChanged(cfgBefore, cfg)) {
    saveToNvs(cfg, true);
  }

  if (!ok) {
    Serial.println("[wifi] falha ao conectar / portal expirou");
    return false;
  }

  Serial.printf("[wifi] conectado SSID=%s IP=%s\n",
                WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str());
  return true;
}
