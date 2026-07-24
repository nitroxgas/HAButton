#include "mqtt_ha.h"
#include "config.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace {

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

constexpr const char* kFwVersion = "1.2.0";
// Prefixo padrao do HA — discovery SO funciona se o broker permitir publish aqui.
constexpr const char* kHaDiscoveryPrefix = "homeassistant";

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

String stateTopic(const AppConfig& cfg, const char* btnKey) {
  return cfg.deviceName + "/" + macSuffix() + "/" + btnKey + "/event";
}

String eventDiscoveryTopic(const char* prefix, const AppConfig& cfg, const char* btnKey) {
  // Formato oficial single-component:
  // <discovery_prefix>/event/<object_id>/config
  // object_id == unique_id (best practice HA)
  return String(prefix) + "/event/" + deviceId(cfg) + "_" + btnKey + "/config";
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

    Serial.printf("[mqtt] falha rc=%d — verifique user/senha/ACL do broker\n", mqtt.state());
    delay(400);
  }
  return false;
}

bool publishRetained(const String& topic, const char* payload, size_t len) {
  const bool ok =
      mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t*>(payload), len, true);
  Serial.printf("[mqtt] RETAIN %s (%u B) -> %s\n", topic.c_str(),
                static_cast<unsigned>(len), ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(80);
  return ok;
}

// Single-component discovery (compativel com HA desde o suporte a MQTT Event ~2023.8).
// Requisitos HA:
// - topico: <prefix>/event/<object_id>/config  (object_id: [a-zA-Z0-9_-])
// - payload JSON retained
// - unique_id (obrigatorio para device registry)
// - state_topic
// - event_types (obrigatorio para event)
// - device.identifiers ou device.connections (para agrupar no device)
// - origin recomendado
bool publishEventDiscovery(const AppConfig& cfg,
                           const char* prefix,
                           const char* btnKey,
                           const String& friendlyName) {
  JsonDocument doc;
  const String uid = deviceId(cfg) + "_" + btnKey;

  doc["name"] = friendlyName;
  doc["unique_id"] = uid;
  doc["object_id"] = cfg.deviceName + "_" + btnKey;
  doc["state_topic"] = stateTopic(cfg, btnKey);
  doc["event_types"][0] = "press";
  doc["device_class"] = "button";
  doc["qos"] = 0;

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
  device["serial_number"] = macSuffix();

  char payload[1024];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.printf("[mqtt] discovery JSON overflow n=%u\n", static_cast<unsigned>(n));
    return false;
  }

  Serial.printf("[mqtt] discovery payload=%s\n", payload);
  return publishRetained(eventDiscoveryTopic(prefix, cfg, btnKey), payload, n);
}

bool publishAllDiscovery(const AppConfig& cfg) {
  // Sempre publica no prefixo canonico do HA.
  bool ok = true;
  ok = publishEventDiscovery(cfg, kHaDiscoveryPrefix, "btn_a", cfg.btnAName) && ok;
  ok = publishEventDiscovery(cfg, kHaDiscoveryPrefix, "btn_b", cfg.btnBName) && ok;

  // Se o portal tiver outro prefixo (HA customizado), publica la tambem.
  String custom = cfg.mqttPrefix;
  custom.trim();
  while (custom.endsWith("/")) {
    custom.remove(custom.length() - 1);
  }
  if (custom.length() > 0 && custom != kHaDiscoveryPrefix) {
    Serial.printf("[mqtt] prefixo custom do portal=%s (tambem publicando)\n", custom.c_str());
    ok = publishEventDiscovery(cfg, custom.c_str(), "btn_a", cfg.btnAName) && ok;
    ok = publishEventDiscovery(cfg, custom.c_str(), "btn_b", cfg.btnBName) && ok;
  }

  return ok;
}

void publishPress(const AppConfig& cfg, const char* btnKey) {
  const String topic = stateTopic(cfg, btnKey);
  // Nao retained: HA descarta retained em entidades event.
  const char* payload = "{\"event_type\":\"press\"}";
  const bool ok = mqtt.publish(topic.c_str(), payload, false);
  Serial.printf("[mqtt] EVENT %s %s -> %s\n", topic.c_str(), payload, ok ? "ok" : "FAIL");
  mqtt.loop();
  delay(50);
}

}  // namespace

bool mqttPublishButtonEvent(const AppConfig& cfg, const ButtonState& buttons) {
  if (!mqttConnect(cfg)) {
    Serial.println("[mqtt] nao conectou");
    return false;
  }

  Serial.println("[mqtt] === HA discovery requirements ===");
  Serial.printf("[mqtt] MUST retain: %s/event/<unique_id>/config\n", kHaDiscoveryPrefix);
  Serial.printf("[mqtt] state/event topics (fora do prefixo): %s/%s/btn_*/event\n",
                cfg.deviceName.c_str(), macSuffix().c_str());
  Serial.println("[mqtt] Broker ACL precisa permitir WRITE em homeassistant/#");

  const bool discOk = publishAllDiscovery(cfg);
  if (!discOk) {
    Serial.println("[mqtt] AVISO: discovery FAIL — confira ACL/topic no MQTT Explorer");
  }

  // Pequena pausa para o HA processar discovery antes do evento.
  delay(200);
  mqtt.loop();

  if (!buttons.a && !buttons.b) {
    Serial.println("[mqtt] sem clique; so discovery");
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
