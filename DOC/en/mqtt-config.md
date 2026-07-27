# Configuration via MQTT / Home Assistant

**Language:** [English](mqtt-config.md) · [Português](../mqtt-config.md)

With the device **awake**, HA (or MQTT Explorer) can change settings. The ESP stores them in NVS.

## From HA (recommended)

After discovery schema **7**, the MQTT device shows (classic per-component discovery):

- Switch **Debug**
- Numbers: **Sleep delay**, **Long press**, **Effect hold**, **Effect mV**
- Texts: button names, device name, OTA password, MQTT host/port/user/password, HA prefix

Change them in the HA UI → ESP receives `{base}/…/set`, applies, and republishes state.

> Only works during the awake session (after a click). There is no subscribe in deep sleep.

## Via JSON (`{base}/config/set`)

| Key | Example |
|-----|---------|
| `debug` | `true` / `false` |
| `sleep_delay_ms` | `20000` |
| `long_press_ms` | `800` |
| `effect_hold_ms` | `500` |
| `effect_target_mv` | `2500` |
| `btn_a_name` / `btn_b_name` / `btn_c_name` | text |
| `device_name` | text |
| `ota_pass` | text |
| `mqtt_host` / `mqtt_port` / `mqtt_user` / `mqtt_pass` / `mqtt_prefix` | broker |

```yaml
alias: HAButton config batch
mode: single
triggers: []
actions:
  - action: mqtt.publish
    data:
      topic: habutton/983dae4191c0/config/set
      payload: '{"debug":true,"sleep_delay_ms":60000,"long_press_ms":900}'
```

## Logs

With Debug=ON → `{base}/log` with JSON (`event`,`t_ms`,`wake`,`ip`,`rssi`,`heap`,`detail`).

## Limitation

MQTT commands only in the awake window. Changing MQTT host/password forces reconnect in the same session.
