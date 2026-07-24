# Configuração MQTT / Home Assistant

## Onde estão os contadores temporais?

Guia completo (hora / dia / semana / mês):

**[DOC/homeassistant-contadores.md](homeassistant-contadores.md)**

Índice geral: [DOC/README.md](README.md).

---

## Dois tipos de tópico (não confundir)

| Tipo | Tópico (exemplo) | Retained | Função |
|------|------------------|----------|--------|
| **Discovery** | `homeassistant/event/habutton_<mac>_btn_a/config` | **sim** | Cria device + entidades no HA |
| **Evento** | `habutton/<mac>/btn_a/event` | **não** | Dispara o clique (`{"event_type":"press"}`) |

Ver só `habutton/...` no broker **não** cria entidades.  
É preciso existir o retained sob `homeassistant/event/.../config`.

## Requisitos oficiais atendidos pelo firmware (v1.2)

Fonte: [MQTT Discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery) e [MQTT Event](https://www.home-assistant.io/integrations/event.mqtt/).

### Discovery (single-component)

1. Tópico: `<prefix>/event/<object_id>/config` com `object_id` ∈ `[a-zA-Z0-9_-]`
2. Payload JSON **retained**
3. Campos obrigatórios / necessários:
   - `state_topic`
   - `event_types` (lista; usamos `press`)
   - `unique_id` (para device registry)
   - `device.identifiers` **ou** `device.connections` (usamos os dois + MAC)
4. Recomendados: `name`, `device_class: button`, `origin`, `qos`
5. Prefixo canônico sempre publicado: **`homeassistant`** (independente de erro no portal)

### Evento de clique

- Payload: `{"event_type":"press"}` (JSON)
- **Não** retained (HA ignora retained em `event`)

### Home Assistant

- Integração MQTT com **Enable discovery** = on  
- Discovery prefix = `homeassistant` (padrão)  
- HA ≥ **2023.8** (entidade `event`)  
- Usuário MQTT com **ACL de WRITE** em `homeassistant/#`  
  (em brokers externos isso costuma ser a causa: estado em `habutton/#` ok, discovery bloqueado)

## Checklist se as entidades não aparecem

1. MQTT Explorer → existe retained em  
   `homeassistant/event/habutton_<mac>_btn_a/config` e `..._btn_b/config`?
2. Serial do ESP mostra `RETAIN homeassistant/event/... -> ok`?  
   Se `FAIL` → ACL/credencial/broker.
3. Em **Configurações → Dispositivos e serviços → MQTT** → devices descobertos / log.
4. Em **Configurações → Sistema → Logs**, filtro `mqtt`, nível debug se preciso.
5. `pio run -t upload`, pressione o botão (cada wake republica discovery).

Entidades esperadas (exemplo):

- Device: `habutton` (manufacturer HAButton)
- `event.habutton_btn_a` / `event.habutton_btn_b`

## Portal WiFiManager

| Campo | Default | Nota |
|-------|---------|------|
| MQTT Host / Port / User / Pass | ver `config.h` / `secrets.h` | Broker |
| HA Discovery Prefix | `homeassistant` | Discovery **sempre** também em `homeassistant`; custom só se o HA usar outro prefixo |
| Device / Botão nomes | `habutton`, `Botao A/B` | Nomes amigáveis |

## Contadores

Depois que `event.*` existir → siga **[homeassistant-contadores.md](homeassistant-contadores.md)**  
(Counter → Template `total_increasing` → Utility Meter hourly/daily/weekly/monthly).
