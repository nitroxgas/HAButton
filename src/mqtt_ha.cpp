#include "mqtt_ha.h"
#include "buttons.h"
#include "config.h"
#include "effect_out.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <esp_sleep.h>

namespace {

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

constexpr const char* kHaDiscoveryPrefix = "homeassistant";

volatile bool g_echoOk = false;
String g_echoTopic;

AppConfig* g_cfgPtr = nullptr;
bool g_configDirty = false;
bool g_subscribed = false;
bool g_needMqttReconnect = false;

const char* kEventTypes[] = {
    "press_a",  "press_b",  "press_c",  "long_a",   "long_b",   "long_c",
    "press_ab", "press_ac", "press_bc", "press_abc",
    "long_ab",  "long_ac",  "long_bc",  "long_abc",
};

String macSuffix() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[13];
  snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String macColon() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String deviceId(const AppConfig& cfg) {
  return cfg.deviceName + "_" + macSuffix();
}

String baseTopic(const AppConfig& cfg) {
  return cfg.deviceName + "/" + macSuffix();
}

// Contrato estável (compatível com packages/HA existentes).
String tEvent(const AppConfig& c) { return baseTopic(c) + "/event"; }
String tLog(const AppConfig& c) { return baseTopic(c) + "/log"; }
String tCfg(const AppConfig& c) { return baseTopic(c) + "/config"; }
String tDbg(const AppConfig& c) { return baseTopic(c) + "/debug"; }
String tDbgSet(const AppConfig& c) { return baseTopic(c) + "/debug/set"; }
String tSlp(const AppConfig& c) { return baseTopic(c) + "/sleep_delay"; }
String tSlpSet(const AppConfig& c) { return baseTopic(c) + "/sleep_delay/set"; }
String tLp(const AppConfig& c) { return baseTopic(c) + "/long_press"; }
String tLpSet(const AppConfig& c) { return baseTopic(c) + "/long_press/set"; }
String tEh(const AppConfig& c) { return baseTopic(c) + "/effect_hold"; }
String tEhSet(const AppConfig& c) { return baseTopic(c) + "/effect_hold/set"; }
String tEmv(const AppConfig& c) { return baseTopic(c) + "/effect_mv"; }
String tEmvSet(const AppConfig& c) { return baseTopic(c) + "/effect_mv/set"; }
String tBa(const AppConfig& c) { return baseTopic(c) + "/btn_a_name"; }
String tBaSet(const AppConfig& c) { return baseTopic(c) + "/btn_a_name/set"; }
String tBb(const AppConfig& c) { return baseTopic(c) + "/btn_b_name"; }
String tBbSet(const AppConfig& c) { return baseTopic(c) + "/btn_b_name/set"; }
String tBc(const AppConfig& c) { return baseTopic(c) + "/btn_c_name"; }
String tBcSet(const AppConfig& c) { return baseTopic(c) + "/btn_c_name/set"; }
String tOta(const AppConfig& c) { return baseTopic(c) + "/ota_pass"; }
String tOtaSet(const AppConfig& c) { return baseTopic(c) + "/ota_pass/set"; }
String tDn(const AppConfig& c) { return baseTopic(c) + "/device_name"; }
String tDnSet(const AppConfig& c) { return baseTopic(c) + "/device_name/set"; }
String tMh(const AppConfig& c) { return baseTopic(c) + "/mqtt_host"; }
String tMhSet(const AppConfig& c) { return baseTopic(c) + "/mqtt_host/set"; }
String tMp(const AppConfig& c) { return baseTopic(c) + "/mqtt_port"; }
String tMpSet(const AppConfig& c) { return baseTopic(c) + "/mqtt_port/set"; }
String tMu(const AppConfig& c) { return baseTopic(c) + "/mqtt_user"; }
String tMuSet(const AppConfig& c) { return baseTopic(c) + "/mqtt_user/set"; }
String tMw(const AppConfig& c) { return baseTopic(c) + "/mqtt_pass"; }
String tMwSet(const AppConfig& c) { return baseTopic(c) + "/mqtt_pass/set"; }
String tPx(const AppConfig& c) { return baseTopic(c) + "/mqtt_prefix"; }
String tPxSet(const AppConfig& c) { return baseTopic(c) + "/mqtt_prefix/set"; }

String eventDiscoveryTopic(const AppConfig& cfg) {
  return String(kHaDiscoveryPrefix) + "/event/" + deviceId(cfg) + "/config";
}

const char* wakeCauseLabel() {
  switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_GPIO:
      return "gpio";
    case ESP_SLEEP_WAKEUP_TIMER:
      return "timer";
    case ESP_SLEEP_WAKEUP_UNDEFINED:
      return "power_on";
    default:
      return "other";
  }
}

bool publishRetained(const String& topic, const char* payload, size_t len) {
  const bool ok =
      mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t*>(payload), len, true);
  Serial.printf("[mqtt] RETAIN %s (%u B) -> %s\n", topic.c_str(),
                static_cast<unsigned>(len), ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(30);
  return ok;
}

bool publishEmptyRetained(const String& topic) {
  const bool ok = mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t*>(""), 0, true);
  Serial.printf("[mqtt] CLEAR %s -> %s\n", topic.c_str(), ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(15);
  return ok;
}

bool verifyRetained(const String& topic) {
  g_echoOk = false;
  g_echoTopic = topic;
  if (!mqtt.subscribe(topic.c_str(), 0)) {
    g_echoTopic = "";
    return false;
  }
  const uint32_t deadline = millis() + 1500;
  while (millis() < deadline && !g_echoOk) {
    mqtt.loop();
    delay(10);
  }
  mqtt.unsubscribe(topic.c_str());
  mqtt.loop();
  g_echoTopic = "";
  Serial.printf("[mqtt] VERIFY %s -> %s\n", topic.c_str(), g_echoOk ? "ok" : "FAIL");
  return g_echoOk;
}

uint8_t storedDiscoveryVersion() {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) {
    return 0;
  }
  const uint8_t ver = prefs.getUChar("disc_ver", 0);
  prefs.end();
  return ver;
}

void markDiscoveryDone(uint8_t ver) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    return;
  }
  prefs.putUChar("disc_ver", ver);
  prefs.putBool("disc_done", true);
  prefs.end();
}

void fillDeviceObject(JsonObject device, const AppConfig& cfg) {
  device["identifiers"][0] = deviceId(cfg);
  device["connections"][0][0] = "mac";
  device["connections"][0][1] = macColon();
  device["name"] = cfg.deviceName;
  device["model"] = "ESP32-C3 Super Mini";
  device["manufacturer"] = "HAButton";
  device["sw_version"] = FW_VERSION;
  const String ip = WiFi.localIP().toString();
  if (ip.length() > 0 && ip != "0.0.0.0") {
    device["configuration_url"] = String("http://") + ip;
    device["connections"][1][0] = "ip";
    device["connections"][1][1] = ip;
  }
}

void clearBrokenCompactDiscovery(const AppConfig& cfg) {
  const String id = deviceId(cfg);
  const String p = kHaDiscoveryPrefix;
  // Schema 6 (device discovery + unique_ids curtos).
  publishEmptyRetained(p + "/device/" + id + "/config");
  publishEmptyRetained(p + "/event/" + id + "_e/config");
  publishEmptyRetained(p + "/switch/" + id + "_dbg/config");
  publishEmptyRetained(p + "/number/" + id + "_slp/config");
  publishEmptyRetained(p + "/number/" + id + "_lp/config");
  publishEmptyRetained(p + "/number/" + id + "_eh/config");
  publishEmptyRetained(p + "/number/" + id + "_emv/config");
  publishEmptyRetained(p + "/text/" + id + "_ba/config");
  publishEmptyRetained(p + "/text/" + id + "_bb/config");
  publishEmptyRetained(p + "/text/" + id + "_bc/config");
  publishEmptyRetained(p + "/text/" + id + "_dn/config");
  publishEmptyRetained(p + "/text/" + id + "_ota/config");
  publishEmptyRetained(p + "/text/" + id + "_mh/config");
  publishEmptyRetained(p + "/text/" + id + "_mp/config");
  publishEmptyRetained(p + "/text/" + id + "_mu/config");
  publishEmptyRetained(p + "/text/" + id + "_mw/config");
  publishEmptyRetained(p + "/text/" + id + "_px/config");
}

void clearLegacyDiscovery(const AppConfig& cfg) {
  const String id = deviceId(cfg);
  const String p = kHaDiscoveryPrefix;
  publishEmptyRetained(p + "/binary_sensor/" + id + "_btn_a/config");
  publishEmptyRetained(p + "/binary_sensor/" + id + "_btn_b/config");
  publishEmptyRetained(p + "/event/" + id + "_btn_a/config");
  publishEmptyRetained(p + "/event/" + id + "_btn_b/config");
  clearBrokenCompactDiscovery(cfg);

  String custom = cfg.mqttPrefix;
  custom.trim();
  if (custom.length() > 0 && custom != kHaDiscoveryPrefix) {
    publishEmptyRetained(custom + "/binary_sensor/" + id + "_btn_a/config");
    publishEmptyRetained(custom + "/binary_sensor/" + id + "_btn_b/config");
    publishEmptyRetained(custom + "/event/" + id + "_btn_a/config");
    publishEmptyRetained(custom + "/event/" + id + "_btn_b/config");
    publishEmptyRetained(custom + "/device/" + id + "/config");
  }
}

bool publishComponentDiscovery(const char* component, const String& objectId,
                               JsonDocument& doc) {
  char payload[1536];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.printf("[mqtt] discovery overflow %s/%s\n", component, objectId.c_str());
    return false;
  }
  const String topic =
      String(kHaDiscoveryPrefix) + "/" + component + "/" + objectId + "/config";
  return publishRetained(topic, payload, n);
}

bool publishEventDiscovery(const AppConfig& cfg) {
  JsonDocument doc;
  doc["name"] = cfg.deviceName;
  doc["unique_id"] = deviceId(cfg) + "_event";
  doc["state_topic"] = tEvent(cfg);
  doc["device_class"] = "button";

  JsonArray types = doc["event_types"].to<JsonArray>();
  for (const char* t : kEventTypes) {
    types.add(t);
  }

  JsonObject origin = doc["origin"].to<JsonObject>();
  origin["name"] = "HAButton";
  origin["sw"] = FW_VERSION;
  fillDeviceObject(doc["device"].to<JsonObject>(), cfg);

  char payload[1280];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.println("[mqtt] event discovery overflow");
    return false;
  }
  Serial.printf("[mqtt] discovery payload=%s\n", payload);
  const String topic = eventDiscoveryTopic(cfg);
  if (!publishRetained(topic, payload, n)) {
    return false;
  }
  return verifyRetained(topic);
}

bool publishSwitchDiscovery(const AppConfig& cfg, const char* name, const String& uniq,
                            const String& stat, const String& cmd, const char* on,
                            const char* off) {
  JsonDocument doc;
  doc["name"] = name;
  doc["unique_id"] = uniq;
  doc["state_topic"] = stat;
  doc["command_topic"] = cmd;
  doc["payload_on"] = on;
  doc["payload_off"] = off;
  doc["entity_category"] = "config";
  fillDeviceObject(doc["device"].to<JsonObject>(), cfg);
  return publishComponentDiscovery("switch", uniq, doc);
}

bool publishNumberDiscovery(const AppConfig& cfg, const char* name, const String& uniq,
                            const String& stat, const String& cmd, int minV, int maxV,
                            int step, const char* unit) {
  JsonDocument doc;
  doc["name"] = name;
  doc["unique_id"] = uniq;
  doc["state_topic"] = stat;
  doc["command_topic"] = cmd;
  doc["min"] = minV;
  doc["max"] = maxV;
  doc["step"] = step;
  doc["unit_of_measurement"] = unit;
  doc["entity_category"] = "config";
  doc["mode"] = "box";
  fillDeviceObject(doc["device"].to<JsonObject>(), cfg);
  return publishComponentDiscovery("number", uniq, doc);
}

bool publishTextDiscovery(const AppConfig& cfg, const char* name, const String& uniq,
                          const String& stat, const String& cmd) {
  JsonDocument doc;
  doc["name"] = name;
  doc["unique_id"] = uniq;
  doc["state_topic"] = stat;
  doc["command_topic"] = cmd;
  doc["entity_category"] = "config";
  fillDeviceObject(doc["device"].to<JsonObject>(), cfg);
  return publishComponentDiscovery("text", uniq, doc);
}

bool publishDiscovery(const AppConfig& cfg) {
  clearLegacyDiscovery(cfg);

  if (!publishEventDiscovery(cfg)) {
    Serial.println("[mqtt] discovery event VERIFY fail (ACL homeassistant/#?)");
    return false;
  }

  const String id = deviceId(cfg);
  publishSwitchDiscovery(cfg, "Debug", id + "_debug", tDbg(cfg), tDbgSet(cfg), "ON", "OFF");
  publishNumberDiscovery(cfg, "Sleep delay", id + "_sleep_delay", tSlp(cfg), tSlpSet(cfg),
                         SLEEP_DELAY_MIN_MS, SLEEP_DELAY_MAX_MS, 1000, "ms");
  publishNumberDiscovery(cfg, "Long press", id + "_long_press", tLp(cfg), tLpSet(cfg), 200,
                         10000, 50, "ms");
  publishNumberDiscovery(cfg, "Effect hold", id + "_effect_hold", tEh(cfg), tEhSet(cfg), 0,
                         10000, 50, "ms");
  publishNumberDiscovery(cfg, "Effect mV", id + "_effect_mv", tEmv(cfg), tEmvSet(cfg), 100,
                         EFFECT_SUPPLY_MV, 50, "mV");
  publishTextDiscovery(cfg, "Btn A name", id + "_btn_a_name", tBa(cfg), tBaSet(cfg));
  publishTextDiscovery(cfg, "Btn B name", id + "_btn_b_name", tBb(cfg), tBbSet(cfg));
  publishTextDiscovery(cfg, "Btn C name", id + "_btn_c_name", tBc(cfg), tBcSet(cfg));
  publishTextDiscovery(cfg, "Device name", id + "_device_name", tDn(cfg), tDnSet(cfg));
  publishTextDiscovery(cfg, "OTA password", id + "_ota_pass", tOta(cfg), tOtaSet(cfg));
  publishTextDiscovery(cfg, "MQTT host", id + "_mqtt_host", tMh(cfg), tMhSet(cfg));
  publishTextDiscovery(cfg, "MQTT port", id + "_mqtt_port", tMp(cfg), tMpSet(cfg));
  publishTextDiscovery(cfg, "MQTT user", id + "_mqtt_user", tMu(cfg), tMuSet(cfg));
  publishTextDiscovery(cfg, "MQTT password", id + "_mqtt_pass", tMw(cfg), tMwSet(cfg));
  publishTextDiscovery(cfg, "HA prefix", id + "_mqtt_prefix", tPx(cfg), tPxSet(cfg));

  markDiscoveryDone(DISCOVERY_SCHEMA_VERSION);
  return true;
}

bool discoveryNeedsPublish() {
  return storedDiscoveryVersion() != DISCOVERY_SCHEMA_VERSION;
}

void pubStr(const String& topic, const String& val) {
  mqtt.publish(topic.c_str(), val.c_str(), true);
}

void pubU32(const String& topic, uint32_t v) {
  char b[12];
  snprintf(b, sizeof(b), "%u", v);
  mqtt.publish(topic.c_str(), b, true);
}

void publishConfigSnapshot(const AppConfig& cfg) {
  JsonDocument doc;
  doc["debug"] = cfg.debugMqttEnabled();
  doc["sleep_delay_ms"] = cfg.sleepDelayMsValue();
  doc["long_press_ms"] = cfg.longPressMsValue();
  doc["effect_hold_ms"] = cfg.effectHoldMsValue();
  doc["effect_target_mv"] = cfg.effectTargetMvValue();
  doc["fw"] = FW_VERSION;
  char payload[220];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n > 0 && n < sizeof(payload)) {
    publishRetained(tCfg(cfg), payload, n);
  }

  pubStr(tDbg(cfg), cfg.debugMqttEnabled() ? "ON" : "OFF");
  pubU32(tSlp(cfg), cfg.sleepDelayMsValue());
  pubU32(tLp(cfg), cfg.longPressMsValue());
  pubU32(tEh(cfg), cfg.effectHoldMsValue());
  pubU32(tEmv(cfg), cfg.effectTargetMvValue());
  pubStr(tBa(cfg), cfg.btnAName);
  pubStr(tBb(cfg), cfg.btnBName);
  pubStr(tBc(cfg), cfg.btnCName);
  pubStr(tDn(cfg), cfg.deviceName);
  pubStr(tOta(cfg), cfg.otaPass.length() > 0 ? "*" : "");
  pubStr(tMh(cfg), cfg.mqttHost);
  pubStr(tMp(cfg), cfg.mqttPort);
  pubStr(tMu(cfg), cfg.mqttUser);
  pubStr(tMw(cfg), cfg.mqttPass.length() > 0 ? "*" : "");
  pubStr(tPx(cfg), cfg.mqttPrefix);
  mqtt.loop();
}

bool setIfChanged(String& field, const String& next) {
  if (field == next) {
    return false;
  }
  field = next;
  return true;
}

bool applyConfigJson(AppConfig& cfg, const char* json, size_t len) {
  JsonDocument doc;
  if (deserializeJson(doc, json, len)) {
    return false;
  }
  bool changed = false;

  if (!doc["debug"].isNull() || !doc["d"].isNull()) {
    String v = !doc["debug"].isNull() ? doc["debug"].as<String>() : doc["d"].as<String>();
    v.trim();
    const String next =
        (v == "1" || v.equalsIgnoreCase("true") || v.equalsIgnoreCase("ON")) ? "1" : "0";
    changed |= setIfChanged(cfg.debugMqtt, next);
  }

  auto take = [&](const char* key, String& field) {
    if (!doc[key].isNull()) {
      changed |= setIfChanged(field, doc[key].as<String>());
    }
  };
  take("sleep_delay_ms", cfg.sleepDelayMs);
  take("long_press_ms", cfg.longPressMs);
  take("effect_hold_ms", cfg.effectHoldMs);
  take("effect_target_mv", cfg.effectTargetMv);
  take("btn_a_name", cfg.btnAName);
  take("btn_b_name", cfg.btnBName);
  take("btn_c_name", cfg.btnCName);
  take("device_name", cfg.deviceName);
  take("ota_pass", cfg.otaPass);
  if (!doc["mqtt_host"].isNull()) {
    changed |= setIfChanged(cfg.mqttHost, doc["mqtt_host"].as<String>());
    g_needMqttReconnect = true;
  }
  if (!doc["mqtt_port"].isNull()) {
    changed |= setIfChanged(cfg.mqttPort, doc["mqtt_port"].as<String>());
    g_needMqttReconnect = true;
  }
  if (!doc["mqtt_user"].isNull()) {
    changed |= setIfChanged(cfg.mqttUser, doc["mqtt_user"].as<String>());
    g_needMqttReconnect = true;
  }
  if (!doc["mqtt_pass"].isNull()) {
    const String v = doc["mqtt_pass"].as<String>();
    if (v != "*") {
      changed |= setIfChanged(cfg.mqttPass, v);
      g_needMqttReconnect = true;
    }
  }
  take("mqtt_prefix", cfg.mqttPrefix);

  if (!changed) {
    return false;
  }
  wifiSaveConfig(cfg);
  wifiApplyRuntimeTuning(cfg);
  publishConfigSnapshot(cfg);
  return true;
}

bool applyScalarSet(AppConfig& cfg, const String& key, const char* val) {
  bool changed = false;
  const String v(val);
  if (key == "debug") {
    const String next =
        (v == "1" || v.equalsIgnoreCase("ON") || v.equalsIgnoreCase("true")) ? "1" : "0";
    changed = setIfChanged(cfg.debugMqtt, next);
  } else if (key == "sleep_delay") {
    changed = setIfChanged(cfg.sleepDelayMs, v);
  } else if (key == "long_press") {
    changed = setIfChanged(cfg.longPressMs, v);
  } else if (key == "effect_hold") {
    changed = setIfChanged(cfg.effectHoldMs, v);
  } else if (key == "effect_mv") {
    changed = setIfChanged(cfg.effectTargetMv, v);
  } else if (key == "btn_a_name") {
    changed = setIfChanged(cfg.btnAName, v);
  } else if (key == "btn_b_name") {
    changed = setIfChanged(cfg.btnBName, v);
  } else if (key == "btn_c_name") {
    changed = setIfChanged(cfg.btnCName, v);
  } else if (key == "device_name") {
    changed = setIfChanged(cfg.deviceName, v);
  } else if (key == "ota_pass") {
    changed = setIfChanged(cfg.otaPass, v);
  } else if (key == "mqtt_host") {
    changed = setIfChanged(cfg.mqttHost, v);
    g_needMqttReconnect = true;
  } else if (key == "mqtt_port") {
    changed = setIfChanged(cfg.mqttPort, v);
    g_needMqttReconnect = true;
  } else if (key == "mqtt_user") {
    changed = setIfChanged(cfg.mqttUser, v);
    g_needMqttReconnect = true;
  } else if (key == "mqtt_pass" && v != "*") {
    changed = setIfChanged(cfg.mqttPass, v);
    g_needMqttReconnect = true;
  } else if (key == "mqtt_prefix") {
    changed = setIfChanged(cfg.mqttPrefix, v);
  } else if (key == "config") {
    return applyConfigJson(cfg, val, strlen(val));
  }
  if (!changed) {
    return false;
  }
  wifiSaveConfig(cfg);
  wifiApplyRuntimeTuning(cfg);
  publishConfigSnapshot(cfg);
  return true;
}

void handleIncoming(char* topic, byte* payload, unsigned int length) {
  if (g_echoTopic.length() > 0 && g_echoTopic.equals(topic) && length > 0) {
    g_echoOk = true;
  }
  if (g_cfgPtr == nullptr) {
    return;
  }

  char buf[256];
  if (length >= sizeof(buf)) {
    length = sizeof(buf) - 1;
  }
  memcpy(buf, payload, length);
  buf[length] = '\0';

  AppConfig& cfg = *g_cfgPtr;
  const String base = baseTopic(cfg) + "/";
  String t(topic);
  if (!t.startsWith(base) || !t.endsWith("/set")) {
    return;
  }
  String mid = t.substring(base.length());
  mid.remove(mid.length() - 4);  // remove "/set"
  if (applyScalarSet(cfg, mid, buf)) {
    g_configDirty = true;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  handleIncoming(topic, payload, length);
}

void subscribeConfigTopics(const AppConfig& cfg) {
  const String sub = baseTopic(cfg) + "/+/set";
  mqtt.subscribe(sub.c_str(), 0);
  g_subscribed = true;
  mqtt.loop();
  Serial.printf("[mqtt] sub %s\n", sub.c_str());
}

}  // namespace

void mqttPublishConfigState(const AppConfig& cfg) {
  if (!mqtt.connected()) {
    return;
  }
  publishConfigSnapshot(cfg);
}

void mqttPublishLog(AppConfig& cfg, const char* event, const char* detail) {
  if (!cfg.debugMqttEnabled()) {
    return;
  }
  if (!mqttEnsureConnected(cfg)) {
    return;
  }

  JsonDocument doc;
  doc["event"] = event;
  doc["t_ms"] = millis();
  doc["wake"] = wakeCauseLabel();
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["heap"] = ESP.getFreeHeap();
  if (detail != nullptr && detail[0] != '\0') {
    doc["detail"] = detail;
  }

  char payload[280];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    return;
  }
  mqtt.publish(tLog(cfg).c_str(), payload, false);
  mqtt.loop();
}

bool mqttEnsureConnected(AppConfig& cfg) {
  g_cfgPtr = &cfg;

  if (mqtt.connected() && !g_needMqttReconnect) {
    mqtt.loop();
    return true;
  }

  if (g_needMqttReconnect && mqtt.connected()) {
    mqtt.disconnect();
  }
  g_needMqttReconnect = false;
  g_subscribed = false;

  const int port = cfg.mqttPort.toInt() > 0 ? cfg.mqttPort.toInt() : 1883;
  mqtt.setServer(cfg.mqttHost.c_str(), port);
  mqtt.setBufferSize(4096);
  mqtt.setKeepAlive(30);
  mqtt.setCallback(mqttCallback);

  const String clientId = deviceId(cfg);
  const uint32_t deadline = millis() + MQTT_CONNECT_TIMEOUT_MS;

  while (!mqtt.connected() && millis() < deadline) {
    Serial.printf("[mqtt] conectando %s:%d\n", cfg.mqttHost.c_str(), port);
    bool ok = false;
    if (cfg.mqttUser.length() > 0) {
      ok = mqtt.connect(clientId.c_str(), cfg.mqttUser.c_str(), cfg.mqttPass.c_str());
    } else {
      ok = mqtt.connect(clientId.c_str());
    }
    if (ok) {
      Serial.println("[mqtt] conectado");
      break;
    }
    Serial.printf("[mqtt] falha rc=%d\n", mqtt.state());
    delay(300);
  }

  if (!mqtt.connected()) {
    return false;
  }

  if (discoveryNeedsPublish()) {
    Serial.printf("[mqtt] discovery schema=%u — republicando\n",
                  static_cast<unsigned>(DISCOVERY_SCHEMA_VERSION));
    publishDiscovery(cfg);
  }

  subscribeConfigTopics(cfg);
  publishConfigSnapshot(cfg);
  return true;
}

void mqttHandle(AppConfig& cfg) {
  g_cfgPtr = &cfg;
  if (!mqtt.connected()) {
    return;
  }
  mqtt.loop();
  if (!g_subscribed) {
    subscribeConfigTopics(cfg);
  }
  if (g_needMqttReconnect) {
    mqttEnsureConnected(cfg);
  }
  if (g_configDirty) {
    g_configDirty = false;
    mqttPublishLog(cfg, "config_changed", nullptr);
  }
}

bool mqttPublishGesture(AppConfig& cfg, const char* eventType) {
  if (eventType == nullptr || eventType[0] == '\0') {
    return false;
  }
  if (!mqttEnsureConnected(cfg)) {
    Serial.println("[mqtt] publish abortado — sem conexao");
    return false;
  }

  JsonDocument doc;
  doc["event_type"] = eventType;
  char payload[64];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    return false;
  }

  const String topic = tEvent(cfg);
  const bool ok = mqtt.publish(topic.c_str(), payload, false);
  Serial.printf("[mqtt] EVENT %s %s -> %s\n", topic.c_str(), payload, ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(30);

  if (cfg.debugMqttEnabled()) {
    mqttPublishLog(cfg, "gesture", eventType);
  }
  return ok;
}

void mqttDisconnect() {
  g_subscribed = false;
  if (mqtt.connected()) {
    mqtt.disconnect();
  }
}
