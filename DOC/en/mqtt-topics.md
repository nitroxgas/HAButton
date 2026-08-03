# MQTT topics

**Language:** [English](mqtt-topics.md) · [Português](../mqtt-topicos.md)

Base: `{device_name}/{mac12}` — e.g. `habutton/983dae4191c0`  
Event discovery: `homeassistant/event/{device_name}_{mac12}/config`  
Discovery schema: **10** (classic + birth `homeassistant/status` to republish after HA restart).

---

## Runtime

| Topic | Dir. | Retain | Payload | Role |
|-------|------|--------|---------|------|
| `{base}/event` | → | no | `{"event_type":"press_a"}` | Gesture |
| `{base}/log` | → | no | debug JSON | Log (if Debug ON) |
| `{base}/config` | → | yes | JSON snapshot | Snapshot |
| `{base}/config/set` | ← | no | JSON | Batch config |
| `{base}/debug` `/set` | ↔ | state yes | `ON` / `OFF` | Debug |
| `{base}/effect_mirror` `/set` | ↔ | state yes | `ON` / `OFF` | Mirror LED→GPIO7 |
| `{base}/sleep_delay` `/set` | ↔ | state yes | ms | Idle sleep |
| `{base}/long_press` `/set` | ↔ | state yes | ms | Long press |
| `{base}/effect_hold` `/set` | ↔ | state yes | ms | Effect hold |
| `{base}/effect_mv` `/set` | ↔ | state yes | mV | Effect target |
| `{base}/btn_a_name` `/set` (and B/C) | ↔ | state yes | text | Button names |
| `{base}/device_name` `/set` | ↔ | state yes | text | Device name |
| `{base}/ota_pass` `/set` | ↔ | state `*` | text | OTA password |
| `{base}/mqtt_host` `/port` `/user` `/pass` `/prefix` + `/set` | ↔ | state yes | MQTT | Broker |

ESP subscribe: `{base}/+/set` + `homeassistant/status` (HA birth → rediscover).  
Only while awake.

### Gestures on `{base}/event`

JSON: `{"event_type":"press_a"}` (also `long_*`, combos `press_ab`, …).

### `{base}/config/set` (example)

```json
{"debug":true,"sleep_delay_ms":60000,"long_press_ms":900}
```

---

## Home Assistant entities (classic discovery)

| Entity | Type | unique_id |
|--------|------|-----------|
| Event (gestures) | `event` | `{id}_event` |
| Debug | `switch` | `{id}_debug` |
| Sleep / Long press / Effect hold / Effect mV | `number` | `{id}_sleep_delay` etc. |
| Names / OTA / MQTT | `text` | `{id}_btn_a_name` etc. |

Configs use `entity_category: config`. Passwords appear as `*` in state.

---

## Gesture flow

1. Identify button → 2. GPIO7 ON → 3. publish `{base}/event` → 4. hold → 5. GPIO7 OFF → 6. idle

Details: [mqtt-config.md](mqtt-config.md).
