# MQTT / Home Assistant configuration

**Language:** [English](configuration.md) · [Português](../configuracao.md)

## MQTT topics

Full list: **[mqtt-topics.md](mqtt-topics.md)**.  
Remote config / debug: **[mqtt-config.md](mqtt-config.md)**.

## Counters and automations in Home Assistant

Full guide — **UI** (Helpers + paste into editor) and **files** (`packages/habutton.yaml`):  
**[homeassistant-counters.md](homeassistant-counters.md)**

## Awake session and sleep

After connecting, the device stays **awake** (default **20 s** idle).

- Each published gesture **restarts** the timer.
- Portal field: **Sleep delay ms (idle / OTA)** — default `20000`.

## Topics

| Type | Example | Retained |
|------|---------|----------|
| Discovery | `homeassistant/event/habutton_<mac>/config` | yes |
| Event | `habutton/<mac>/event` | no |

Event payload: `{"event_type":"press_a"}` (or `long_b`, `press_ab`, `press_abc`, etc.).

In discovery, the `device` block includes local IP:
- `configuration_url`: `http://<IP>` (link on the HA device)
- `connections`: `["mac", …]` and `["ip", "<IP>"]`

### event_types

`press_a`, `press_b`, `press_c`, `long_a`, `long_b`, `long_c`,  
`press_ab`, `press_ac`, `press_bc`, `press_abc`,  
`long_ab`, `long_ac`, `long_bc`, `long_abc`

A single `event` entity on the HA device.

## WiFiManager portal

| Field | Default |
|-------|---------|
| MQTT Host/Port/User/Pass | see `config.h` / `secrets.h` |
| HA Discovery Prefix | `homeassistant` |
| Device / Buttons A/B/C | `habutton`, friendly names |
| Sleep delay ms | `20000` |
| OTA Password | `habutton-ota` |
| Debug MQTT logs (0/1) | `0` |
| Long press ms | `800` |
| Effect hold ms (min) | `500` |
| Effect target mV | `2500` |

Remote config from HA (debug switch, sleep delay, JSON): **[mqtt-config.md](mqtt-config.md)**.

### Reopen the portal

Hold **A+B+C ~10 s** → AP `HAButton-XXXX` → `http://192.168.4.1`.

## Discovery / ACL

Broker needs WRITE on `homeassistant/#`.  
Serial: `VERIFY ... -> ok`. YAML fallback: [homeassistant-fallback-yaml.md](homeassistant-fallback-yaml.md).

## Home Assistant — what to expect

Firmware publishes **one** MQTT Event entity (`event.habutton_…_event`), not two buttons.

- **Event** domain (since HA **2023.8**). No YAML needed if discovery works.
- Entity has **no continuous state**: “state” becomes a **timestamp** on each gesture; type is in **attributes → `event_type`** (`press_a`, `long_b`, …).
- Older firmware created **2 entities** (`…_btn_a` / `…_btn_b`) on different topics. Firmware **1.3.1+** clears those retained discoveries and republishes the new schema.

### If you still see 2 buttons without updating

1. Flash **1.3.1+** and wake the device (gesture) — serial should show `discovery schema=…` and `CLEAR …/btn_a/config`.
2. In **Devices & services → MQTT → habutton**: remove orphan `btn_a` / `btn_b` entities if they remain.
3. In **Developer tools → States**, confirm `event.…` and the `event_type` attribute after a click.
4. In MQTT Explorer: topic `habutton/<mac>/event` with `{"event_type":"press_a"}` (not retained).

## OTA

Full guide: **[ota.md](ota.md)**.  
During the awake session (and each click that extends idle) `ArduinoOTA` stays active on the LAN.
