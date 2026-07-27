# HAButton Documentation (English)

**Language:** [English](README.md) · [Português](../README.md)

| Document | Contents |
|----------|----------|
| [architecture.md](architecture.md) | Awake session, modules, MQTT |
| [hardware.md](hardware.md) | Pinout A/B/C, GPIO7 effect, portal |
| [configuration.md](configuration.md) | Portal, 20 s sleep, event_types |
| [mqtt-topics.md](mqtt-topics.md) | All MQTT topics (runtime + discovery) |
| [mqtt-config.md](mqtt-config.md) | Debug logs + remote config via MQTT/HA |
| [homeassistant-counters.md](homeassistant-counters.md) | Counters/automations: UI or YAML packages |
| [homeassistant-fallback-yaml.md](homeassistant-fallback-yaml.md) | YAML if discovery fails |
| [development.md](development.md) | USB build overview |
| [ota.md](ota.md) | Full OTA update guide |

## Goal

Battery-powered device with 3 buttons: wakes, keeps an awake session (20 s idle), publishes MQTT gestures with Home Assistant discovery, supports LAN OTA, drives PWM on GPIO7 during publish, then returns to deep sleep.
