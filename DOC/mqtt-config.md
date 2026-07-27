# Configuração via MQTT / Home Assistant

**Idioma:** [Português](mqtt-config.md) · [English](en/mqtt-config.md)


Com o device **acordado**, o HA (ou MQTT Explorer) altera configs. O ESP grava na NVS.

## Pelo HA (recomendado)

Após discovery schema **7**, no device MQTT aparecem (discovery clássico por componente):

- Switch **Debug**
- Numbers: **Sleep delay**, **Long press**, **Effect hold**, **Effect mV**
- Texts: nomes dos botões, device name, OTA password, MQTT host/port/user/password, HA prefix

Altere na UI do HA → o ESP recebe em `{base}/…/set`, aplica e republica o estado.

> Só funciona na sessão acordada (após clique). Em deep sleep não há subscribe.

## Por JSON (`{base}/config/set`)

| Chave | Exemplo |
|-------|---------|
| `debug` | `true` / `false` |
| `sleep_delay_ms` | `20000` |
| `long_press_ms` | `800` |
| `effect_hold_ms` | `500` |
| `effect_target_mv` | `2500` |
| `btn_a_name` / `btn_b_name` / `btn_c_name` | texto |
| `device_name` | texto |
| `ota_pass` | texto |
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

Com Debug=ON → `{base}/log` com JSON (`event`,`t_ms`,`wake`,`ip`,`rssi`,`heap`,`detail`).

## Limitação

Comandos MQTT só na janela acordada. Mudança de MQTT host/senha força reconnect na mesma sessão.
