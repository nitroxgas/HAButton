#include "mqtt_ha.h"
#include "config.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace {

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

constexpr const char* kHaDiscoveryPrefix = "homeassistant";

volatile bool g_echoOk = false;
String g_echoTopic;

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

String stateTopic(const AppConfig& cfg) {
  return cfg.deviceName + "/" + macSuffix() + "/event";
}

String eventDiscoveryTopic(const AppConfig& cfg) {
  return String(kHaDiscoveryPrefix) + "/event/" + deviceId(cfg) + "/config";
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  (void)payload;
  if (g_echoTopic.length() > 0 && g_echoTopic.equals(topic) && length > 0) {
    g_echoOk = true;
  }
}

bool publishRetained(const String& topic, const char* payload, size_t len) {
  const bool ok =
      mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t*>(payload), len, true);
  Serial.printf("[mqtt] RETAIN %s (%u B) -> %s\n", topic.c_str(),
                static_cast<unsigned>(len), ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(40);
  return ok;
}

bool publishEmptyRetained(const String& topic) {
  // Payload vazio + retain remove discovery antigo no HA.
  const bool ok = mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t*>(""), 0, true);
  Serial.printf("[mqtt] CLEAR %s -> %s\n", topic.c_str(), ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(20);
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

void clearLegacyUnderPrefix(const String& prefix, const String& id) {
  publishEmptyRetained(prefix + "/binary_sensor/" + id + "_btn_a/config");
  publishEmptyRetained(prefix + "/binary_sensor/" + id + "_btn_b/config");
  publishEmptyRetained(prefix + "/event/" + id + "_btn_a/config");
  publishEmptyRetained(prefix + "/event/" + id + "_btn_b/config");
}

void clearLegacyDiscovery(const AppConfig& cfg) {
  const String id = deviceId(cfg);
  clearLegacyUnderPrefix(kHaDiscoveryPrefix, id);

  String custom = cfg.mqttPrefix;
  custom.trim();
  if (custom.length() > 0 && custom != kHaDiscoveryPrefix) {
    clearLegacyUnderPrefix(custom, id);
  }
}

bool publishDiscovery(const AppConfig& cfg) {
  clearLegacyDiscovery(cfg);

  JsonDocument doc;
  doc["name"] = cfg.deviceName;
  doc["unique_id"] = deviceId(cfg) + "_event";
  doc["state_topic"] = stateTopic(cfg);
  doc["device_class"] = "button";

  JsonArray types = doc["event_types"].to<JsonArray>();
  for (const char* t : kEventTypes) {
    types.add(t);
  }

  JsonObject origin = doc["origin"].to<JsonObject>();
  origin["name"] = "HAButton";
  origin["sw"] = FW_VERSION;

  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"][0] = deviceId(cfg);
  device["connections"][0][0] = "mac";
  device["connections"][0][1] = macColon();
  device["name"] = cfg.deviceName;
  device["model"] = "ESP32-C3 Super Mini";
  device["manufacturer"] = "HAButton";
  device["sw_version"] = FW_VERSION;

  char payload[1024];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.println("[mqtt] discovery JSON overflow");
    return false;
  }

  Serial.printf("[mqtt] discovery payload=%s\n", payload);
  const String topic = eventDiscoveryTopic(cfg);
  if (!publishRetained(topic, payload, n)) {
    return false;
  }
  if (verifyRetained(topic)) {
    markDiscoveryDone(DISCOVERY_SCHEMA_VERSION);
    return true;
  }
  Serial.println("[mqtt] discovery nao confirmado (ACL homeassistant/#?)");
  return false;
}

bool discoveryNeedsPublish() {
  return storedDiscoveryVersion() != DISCOVERY_SCHEMA_VERSION;
}

}  // namespace

bool mqttEnsureConnected(const AppConfig& cfg) {
  if (mqtt.connected()) {
    mqtt.loop();
    return true;
  }

  const int port = cfg.mqttPort.toInt() > 0 ? cfg.mqttPort.toInt() : 1883;
  mqtt.setServer(cfg.mqttHost.c_str(), port);
  mqtt.setBufferSize(2048);
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
  return true;
}

bool mqttPublishGesture(const AppConfig& cfg, const char* eventType) {
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

  const String topic = stateTopic(cfg);
  const bool ok = mqtt.publish(topic.c_str(), payload, false);
  Serial.printf("[mqtt] EVENT %s %s -> %s\n", topic.c_str(), payload, ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(30);
  return ok;
}

void mqttDisconnect() {
  if (mqtt.connected()) {
    mqtt.disconnect();
  }
}
