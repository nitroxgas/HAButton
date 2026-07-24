#include "mqtt_ha.h"
#include "config.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace {

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

constexpr const char* kFwVersion = "1.1.0";

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

// Prefixo do MQTT Discovery do HA (obrigatorio = homeassistant por padrao).
// Tópicos de ESTADO ficam fora desse prefixo — isso e intencional.
String discoveryPrefix(const AppConfig& cfg) {
  String p = cfg.mqttPrefix;
  p.trim();
  if (p.isEmpty()) {
    p = DEFAULT_MQTT_PREFIX;
  }
  while (p.endsWith("/")) {
    p.remove(p.length() - 1);
  }
  return p;
}

String stateTopic(const AppConfig& cfg, const char* btnKey) {
  return cfg.deviceName + "/" + macSuffix() + "/" + btnKey + "/event";
}

String deviceDiscoveryTopic(const AppConfig& cfg) {
  return discoveryPrefix(cfg) + "/device/" + deviceId(cfg) + "/config";
}

String legacyBinaryDiscoveryTopic(const AppConfig& cfg, const char* btnKey) {
  return discoveryPrefix(cfg) + "/binary_sensor/" + deviceId(cfg) + "_" + btnKey +
         "/config";
}

bool mqttConnect(const AppConfig& cfg) {
  const int port = cfg.mqttPort.toInt() > 0 ? cfg.mqttPort.toInt() : 1883;
  mqtt.setServer(cfg.mqttHost.c_str(), port);
  mqtt.setBufferSize(2048);
  mqtt.setKeepAlive(30);

  const String clientId = deviceId(cfg);
  const uint32_t deadline = millis() + MQTT_CONNECT_TIMEOUT_MS;

  while (!mqtt.connected() && millis() < deadline) {
    Serial.printf("[mqtt] conectando em %s:%d como %s\n",
                  cfg.mqttHost.c_str(), port, clientId.c_str());

    bool connected = false;
    if (cfg.mqttUser.length() > 0) {
      connected = mqtt.connect(clientId.c_str(),
                               cfg.mqttUser.c_str(),
                               cfg.mqttPass.c_str());
    } else {
      connected = mqtt.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("[mqtt] conectado");
      return true;
    }

    Serial.printf("[mqtt] falha rc=%d\n", mqtt.state());
    delay(400);
  }
  return false;
}

void clearLegacyBinaryDiscovery(const AppConfig& cfg) {
  // Remove discovery antigo (binary_sensor) se ainda estiver retained no broker.
  const char* keys[] = {"btn_a", "btn_b"};
  for (const char* key : keys) {
    const String topic = legacyBinaryDiscoveryTopic(cfg, key);
    const bool ok = mqtt.publish(topic.c_str(), "", true);
    Serial.printf("[mqtt] limpa legacy %s -> %s\n", topic.c_str(), ok ? "ok" : "fail");
    mqtt.loop();
  }
}

bool publishDeviceDiscovery(const AppConfig& cfg) {
  JsonDocument doc;

  JsonObject origin = doc["origin"].to<JsonObject>();
  origin["name"] = "HAButton";
  origin["sw"] = kFwVersion;

  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"][0] = deviceId(cfg);
  device["connections"][0][0] = "mac";
  device["connections"][0][1] = macColon();
  device["name"] = cfg.deviceName;
  device["model"] = "ESP32-C3 Super Mini";
  device["manufacturer"] = "HAButton";
  device["sw_version"] = kFwVersion;

  JsonObject cmps = doc["components"].to<JsonObject>();

  auto addEvent = [&](const char* key, const String& name) {
    JsonObject cmp = cmps[key].to<JsonObject>();
    cmp["platform"] = "event";
    cmp["name"] = name;
    cmp["unique_id"] = deviceId(cfg) + "_" + key;
    cmp["device_class"] = "button";
    cmp["event_types"][0] = "press";
    cmp["state_topic"] = stateTopic(cfg, key);
    cmp["default_entity_id"] = String("event.") + cfg.deviceName + "_" + key;
  };

  addEvent("btn_a", cfg.btnAName);
  addEvent("btn_b", cfg.btnBName);

  char payload[1536];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.printf("[mqtt] discovery JSON overflow n=%u\n", static_cast<unsigned>(n));
    return false;
  }

  const String topic = deviceDiscoveryTopic(cfg);
  Serial.printf("[mqtt] discovery topic=%s bytes=%u\n", topic.c_str(),
                static_cast<unsigned>(n));
  Serial.printf("[mqtt] discovery payload=%s\n", payload);

  const bool ok = mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t*>(payload),
                               n, true);
  Serial.printf("[mqtt] discovery publish -> %s\n", ok ? "ok" : "fail");
  mqtt.loop();
  delay(150);
  return ok;
}

void publishPress(const AppConfig& cfg, const char* btnKey) {
  const String topic = stateTopic(cfg, btnKey);
  // MQTT Event exige JSON com event_type; retained=false (HA descarta retained em events).
  const char* payload = "{\"event_type\":\"press\"}";
  const bool ok = mqtt.publish(topic.c_str(), payload, false);
  Serial.printf("[mqtt] event %s %s -> %s\n", topic.c_str(), payload, ok ? "ok" : "fail");
  mqtt.loop();
}

}  // namespace

bool mqttPublishButtonEvent(const AppConfig& cfg, const ButtonState& buttons) {
  if (!mqttConnect(cfg)) {
    Serial.println("[mqtt] nao conectou");
    return false;
  }

  Serial.printf("[mqtt] discovery_prefix=%s state_base=%s/%s/\n",
                discoveryPrefix(cfg).c_str(),
                cfg.deviceName.c_str(),
                macSuffix().c_str());

  clearLegacyBinaryDiscovery(cfg);
  const bool discOk = publishDeviceDiscovery(cfg);
  if (!discOk) {
    Serial.println("[mqtt] AVISO: discovery falhou — HA nao criara o dispositivo");
  }

  if (!buttons.a && !buttons.b) {
    Serial.println("[mqtt] sem acionamento de botao; discovery republicado");
  } else {
    if (buttons.a) {
      publishPress(cfg, "btn_a");
    }
    if (buttons.b) {
      publishPress(cfg, "btn_b");
    }
  }

  const uint32_t until = millis() + MQTT_POST_PUBLISH_MS;
  while (millis() < until) {
    mqtt.loop();
    delay(10);
  }

  mqtt.disconnect();
  return discOk;
}
