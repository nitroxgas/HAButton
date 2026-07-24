# Configuração

## Portal WiFiManager

No **primeiro boot** ou quando **não conseguir conectar** ao Wi‑Fi já salvo:

1. O dispositivo sobe um AP chamado `HAButton-XXXX` (XXXX = finais do MAC).
2. Conecte o celular/PC a esse AP.
3. Abra o portal (geralmente `http://192.168.4.1`).
4. Informe SSID/senha da rede e os parâmetros MQTT abaixo.
5. Salve — o ESP segue para MQTT e deep sleep.

Timeouts (`include/config.h`):

- Tentativa de conexão: ~50 s (`WIFI_CONNECT_TIMEOUT_S`).
- Portal aberto: ~90 s (`WIFI_PORTAL_TIMEOUT_S`); depois dorme.

## Parâmetros (defaults no código)

| Campo no portal | Default | Uso |
|-----------------|---------|-----|
| MQTT Host | `homeassistant.local` | Broker |
| MQTT Port | `1883` | Porta TCP |
| MQTT User | *(vazio)* | Opcional |
| MQTT Password | *(vazio)* | Opcional |
| HA Discovery Prefix | `homeassistant` | **Obrigatório** para auto-discovery |
| Device Name | `habutton` | Nome do device / base dos tópicos de estado |
| Botão A Nome | `Botao A` | Nome amigável no HA |
| Botão B Nome | `Botao B` | Nome amigável no HA |

Defaults em `include/config.h`. Opcionalmente `include/secrets.h` (gitignore) para sobrescrever em compile-time.

## Dois tipos de tópico (importante)

O Home Assistant **só** escuta discovery no prefixo configurado (padrão `homeassistant`).  
Os tópicos de **estado/evento** ficam **fora** desse prefixo — isso é normal e recomendado.

| Tipo | Exemplo | Retained |
|------|---------|----------|
| Discovery (cria o device) | `homeassistant/device/habutton_<mac>/config` | sim |
| Evento do botão | `habutton/<mac>/btn_a/event` | **não** |

Se no MQTT Explorer você só vê `habutton/.../event` (ou `/state` antigo) e **não** vê nada sob `homeassistant/device/.../config`, o HA **não** criará o dispositivo.

### Checklist se o HA não criar o device

1. Integração MQTT no HA com **Enable discovery** ligado e prefixo `homeassistant`.
2. No broker, confirme retained em:  
   `homeassistant/device/habutton_<mac>/config`  
   com JSON contendo `origin`, `device` e `components`.
3. No Serial Monitor, ao acordar, deve aparecer:  
   `[mqtt] discovery topic=homeassistant/device/...` e `discovery publish -> ok`.
4. Prefixo no portal **não** pode estar vazio ou diferente do HA.
5. Após atualizar o firmware, pressione o botão (cada wake republica o discovery).

## Entidades criadas

A partir da v1.1 o firmware usa **MQTT Device Discovery** com entidades `event` (`device_class: button`):

- `event.habutton_btn_a`
- `event.habutton_btn_b`

Payload de acionamento (não retained):

```json
{"event_type":"press"}
```

Legacy `binary_sensor` discovery é limpo automaticamente (payload vazio retained).

## Contadores no Home Assistant

Veja o guia completo: [homeassistant-contadores.md](homeassistant-contadores.md).

## Reabrir o portal

1. `pio run -t erase` e regravar, ou  
2. Forçar falha de Wi‑Fi e aguardar o portal no próximo wake.
