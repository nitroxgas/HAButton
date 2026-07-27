# Configuração MQTT / Home Assistant

**Idioma:** [Português](configuracao.md) · [English](en/configuration.md)

## Tópicos MQTT

Lista completa: **[mqtt-topicos.md](mqtt-topicos.md)**.  
Config remota / debug: **[mqtt-config.md](mqtt-config.md)**.

## Contadores e automações no Home Assistant

Guia completo — **UI** (Helpers + colar no editor) e **arquivos** (`packages/habutton.yaml`):  
**[homeassistant-contadores.md](homeassistant-contadores.md)**

## Sessão acordada e sleep

Após conectar, o device fica **acordado** (default **20 s** de idle).

- Cada gesto publicado **reinicia** o timer.
- Campo portal: **Sleep delay ms (idle / OTA)** — default `20000`.

## Tópicos

| Tipo | Exemplo | Retained |
|------|---------|----------|
| Discovery | `homeassistant/event/habutton_<mac>/config` | sim |
| Evento | `habutton/<mac>/event` | não |

Payload evento: `{"event_type":"press_a"}` (ou `long_b`, `press_ab`, `press_abc`, etc.).

No discovery, o bloco `device` inclui IP local:
- `configuration_url`: `http://<IP>` (link no device no HA)
- `connections`: `["mac", …]` e `["ip", "<IP>"]`

### event_types

`press_a`, `press_b`, `press_c`, `long_a`, `long_b`, `long_c`,  
`press_ab`, `press_ac`, `press_bc`, `press_abc`,  
`long_ab`, `long_ac`, `long_bc`, `long_abc`

Uma única entidade `event` no device HA.

## Portal WiFiManager

| Campo | Default |
|-------|---------|
| MQTT Host/Port/User/Pass | ver `config.h` / `secrets.h` |
| HA Discovery Prefix | `homeassistant` |
| Device / Botões A/B/C | `habutton`, nomes amigáveis |
| Sleep delay ms | `20000` |
| OTA Password | `habutton-ota` |
| Debug MQTT logs (0/1) | `0` |
| Long press ms | `800` |
| Effect hold ms (min) | `500` |
| Effect target mV | `2500` |

Config remota pelo HA (switch debug, sleep delay, JSON): **[mqtt-config.md](mqtt-config.md)**.

### Reabrir o portal

Segure **A+B+C ~10 s** → AP `HAButton-XXXX` → `http://192.168.4.1`.

## Discovery / ACL

Broker precisa de WRITE em `homeassistant/#`.  
Serial: `VERIFY ... -> ok`. Fallback YAML: [homeassistant-fallback-yaml.md](homeassistant-fallback-yaml.md).

## Home Assistant — o que esperar

O firmware publica **uma** entidade MQTT Event (`event.habutton_…_event`), não dois botões.

- Domínio **Event** (desde HA **2023.8**). Não precisa de YAML se o discovery funcionar.
- A entidade é **sem estado contínuo**: o “estado” vira um **timestamp** a cada gesto; o tipo fica em **atributos → `event_type`** (`press_a`, `long_b`, …).
- Versões antigas do firmware criavam **2 entidades** (`…_btn_a` / `…_btn_b`) em tópicos diferentes. O firmware **1.3.1+** limpa esses discovery retained e republica o schema novo.

### Se ainda vir 2 botões sem atualizar

1. Flash **1.3.1+** e acorde o device (gesto) — no serial deve aparecer `discovery schema=…` e `CLEAR …/btn_a/config`.
2. Em **Dispositivos e serviços → MQTT → habutton**: remova entidades órfãs `btn_a` / `btn_b` se sobrarem.
3. Confirme em **Ferramentas de desenvolvedor → Estados** a entidade `event.…` e o atributo `event_type` após um clique.
4. No MQTT Explorer: tópico `habutton/<mac>/event` com `{"event_type":"press_a"}` (não retained).

## OTA

Guia completo: **[ota.md](ota.md)**.  
Durante a sessão acordada (e a cada clique que estende o idle) o `ArduinoOTA` fica ativo na LAN.
