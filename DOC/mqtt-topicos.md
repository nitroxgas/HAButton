# Tópicos MQTT

**Idioma:** [Português](mqtt-topicos.md) · [English](en/mqtt-topics.md)


Base: `{device_name}/{mac12}` — ex. `habutton/983dae4191c0`  
Discovery event: `homeassistant/event/{device_name}_{mac12}/config`  
Schema discovery: **7** (rollback do schema 6 — contrato clássico compatível com HA).

---

## Runtime

| Tópico | Dir. | Retain | Payload | Função |
|--------|------|--------|---------|--------|
| `{base}/event` | → | não | `{"event_type":"press_a"}` | Gesto |
| `{base}/log` | → | não | JSON debug | Log (se Debug ON) |
| `{base}/config` | → | sim | JSON snapshot | Snapshot |
| `{base}/config/set` | ← | não | JSON | Batch config |
| `{base}/debug` `/set` | ↔ | estado sim | `ON` / `OFF` | Debug |
| `{base}/sleep_delay` `/set` | ↔ | estado sim | ms | Sleep idle |
| `{base}/long_press` `/set` | ↔ | estado sim | ms | Long press |
| `{base}/effect_hold` `/set` | ↔ | estado sim | ms | Effect hold |
| `{base}/effect_mv` `/set` | ↔ | estado sim | mV | Effect alvo |
| `{base}/btn_a_name` `/set` (e B/C) | ↔ | estado sim | texto | Nomes botões |
| `{base}/device_name` `/set` | ↔ | estado sim | texto | Device name |
| `{base}/ota_pass` `/set` | ↔ | estado `*` | texto | OTA password |
| `{base}/mqtt_host` `/port` `/user` `/pass` `/prefix` + `/set` | ↔ | estado sim | MQTT | Broker |

Subscribe no ESP: `{base}/+/set` (só enquanto acordado).

### Gestos em `{base}/event`

JSON: `{"event_type":"press_a"}` (também `long_*`, combos `press_ab`, …).

### `{base}/config/set` (exemplo)

```json
{"debug":true,"sleep_delay_ms":60000,"long_press_ms":900}
```

---

## Entidades no Home Assistant (discovery clássico)

| Entidade | Tipo | unique_id |
|----------|------|-----------|
| Event (gestos) | `event` | `{id}_event` |
| Debug | `switch` | `{id}_debug` |
| Sleep / Long press / Effect hold / Effect mV | `number` | `{id}_sleep_delay` etc. |
| Nomes / OTA / MQTT | `text` | `{id}_btn_a_name` etc. |

Configs com `entity_category: config`. Senhas no estado aparecem como `*`.

---

## Fluxo gesto

1. Identifica botão → 2. GPIO7 ON → 3. publica `{base}/event` → 4. hold → 5. GPIO7 OFF → 6. idle

Detalhes: [mqtt-config.md](mqtt-config.md).
