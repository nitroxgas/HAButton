#include "mqtt_ha.h"
#include "config.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace {

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

String macSuffix() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[13];
  snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String deviceId(const AppConfig& cfg) {
  return cfg.deviceName + "_" + macSuffix();
}

String stateTopic(const AppConfig& cfg, const char* btnKey) {
  return cfg.deviceName + "/" + macSuffix() + "/" + btnKey + "/state";
}

String discoveryTopic(const AppConfig& cfg, const char* btnKey) {
  return cfg.mqttPrefix + "/binary_sensor/" + deviceId(cfg) + "_" + btnKey + "/config";
}

bool discoveryAlreadyDone() {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) {
    return false;
  }
  const bool done = prefs.getBool("disc_done", false);
  prefs.end();
  return done;
}

void markDiscoveryDone() {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    return;
  }
  prefs.putBool("disc_done", true);
  prefs.end();
}

bool mqttConnect(const AppConfig& cfg) {
  const int port = cfg.mqttPort.toInt() > 0 ? cfg.mqttPort.toInt() : 1883;
  mqtt.setServer(cfg.mqttHost.c_str(), port);
  mqtt.setBufferSize(1024);
  mqtt.setKeepAlive(15);

  const String clientId = deviceId(cfg);
  const uint32_t deadline = millis() + MQTT_CONNECT_TIMEOUT_MS;

  while (!mqtt.connected() && millis() < deadline) {
    Serial.printf("[mqtt] conectando em %s:%d como %s\n",
                  cfg.mqttHost.c_str(), port, clientId.c_str());

    // Sem LWT/availability: o dispositivo entra em deep sleep e nao deve
    // aparecer como "unavailable" no HA entre acionamentos.
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

void publishDiscoveryEntity(const AppConfig& cfg,
                            const char* btnKey,
                            const String& friendlyName) {
  JsonDocument doc;
  doc["name"] = friendlyName;
  doc["unique_id"] = deviceId(cfg) + "_" + btnKey;
  doc["state_topic"] = stateTopic(cfg, btnKey);
  doc["payload_on"] = "ON";
  doc["payload_off"] = "OFF";
  doc["expire_after"] = 10;
  doc["off_delay"] = 1;

  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"][0] = deviceId(cfg);
  device["name"] = cfg.deviceName;
  device["model"] = "ESP32-C3 Super Mini";
  device["manufacturer"] = "HAButton";
  device["sw_version"] = "1.0.0";

  char payload[768];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.println("[mqtt] discovery JSON overflow");
    return;
  }

  const String topic = discoveryTopic(cfg, btnKey);
  const bool ok = mqtt.publish(topic.c_str(), payload, true);
  Serial.printf("[mqtt] discovery %s -> %s\n", topic.c_str(), ok ? "ok" : "fail");
}

void publishDiscovery(const AppConfig& cfg) {
  publishDiscoveryEntity(cfg, "btn_a", cfg.btnAName);
  publishDiscoveryEntity(cfg, "btn_b", cfg.btnBName);
  markDiscoveryDone();
}

void publishPress(const AppConfig& cfg, const char* btnKey) {
  const String topic = stateTopic(cfg, btnKey);
  // Pulso ON -> OFF para automacoes no HA (binary_sensor)
  bool ok = mqtt.publish(topic.c_str(), "ON", false);
  Serial.printf("[mqtt] %s ON -> %s\n", topic.c_str(), ok ? "ok" : "fail");
  mqtt.loop();
  delay(50);
  ok = mqtt.publish(topic.c_str(), "OFF", false);
  Serial.printf("[mqtt] %s OFF -> %s\n", topic.c_str(), ok ? "ok" : "fail");
}

}  // namespace

bool mqttPublishButtonEvent(const AppConfig& cfg, const ButtonState& buttons) {
  if (!mqttConnect(cfg)) {
    Serial.println("[mqtt] nao conectou");
    return false;
  }

  if (!discoveryAlreadyDone()) {
    publishDiscovery(cfg);
    mqtt.loop();
    delay(100);
  }

  // Power-on / reset sem botao: so garante discovery e volta a dormir
  if (!buttons.a && !buttons.b) {
    Serial.println("[mqtt] sem acionamento de botao; so discovery/conexao");
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
  return true;
}
