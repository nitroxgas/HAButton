# Configuração

## Portal WiFiManager

No **primeiro boot** ou quando **não conseguir conectar** ao Wi‑Fi já salvo:

1. O dispositivo sobe um AP chamado `HAButton-XXXX` (XXXX = finais do MAC).
2. Conecte o celular/PC a esse AP.
3. Abra o portal (geralmente `http://192.168.4.1`).
4. Informe SSID/senha da rede e os parâmetros MQTT abaixo.
5. Salve — o ESP reinicia o fluxo, conecta e segue para MQTT.

Timeouts (`include/config.h`):

- Tentativa de conexão: ~25 s (`WIFI_CONNECT_TIMEOUT_S`).
- Portal aberto: ~90 s (`WIFI_PORTAL_TIMEOUT_S`); depois dorme.

## Parâmetros (defaults no código)

| Campo no portal | Default | Uso |
|-----------------|---------|-----|
| MQTT Host | `homeassistant.local` | Broker |
| MQTT Port | `1883` | Porta TCP |
| MQTT User | *(vazio)* | Opcional |
| MQTT Password | *(vazio)* | Opcional |
| HA Discovery Prefix | `homeassistant` | Prefixo padrão do HA |
| Device Name | `habutton` | Nome / base de tópicos |
| Botão A Nome | `Botao A` | Nome amigável no HA |
| Botão B Nome | `Botao B` | Nome amigável no HA |

Defaults compilados em `include/config.h`. Opcionalmente use `include/secrets.h.example` → `secrets.h` para sobrescrever em compile-time (arquivo ignorado pelo Git).

## Home Assistant

Pré-requisito: integração **MQTT** ativa no HA (Addon Mosquitto ou broker externo) com discovery habilitado.

### Tópicos (exemplo)

Device id: `habutton_<macsemdois pontos>`

- Discovery (retained):  
  `homeassistant/binary_sensor/habutton_<mac>_btn_a/config`  
  `homeassistant/binary_sensor/habutton_<mac>_btn_b/config`
- Estado:  
  `habutton/<mac>/btn_a/state`  
  `habutton/<mac>/btn_b/state`

Payload de acionamento: pulso `ON` seguido de `OFF`.

### O que aparece no HA

Dois `binary_sensor` no mesmo device (`ESP32-C3 Super Mini` / manufacturer `HAButton`). Automations tipicamente usam:

- trigger: estado do sensor muda para `on`.

Após mudar host/prefix/nomes no portal, o firmware republica o discovery (flag NVS `disc_done` é limpa).

## Reabrir o portal

Opções práticas:

1. Apagar credenciais Wi‑Fi (erase flash / `pio run -t erase`) e regravar.
2. Forçar falha de Wi‑Fi (rede inexistente) e aguardar o portal no próximo wake.
3. (Futuro) botão longo / pin de “config” — não implementado nesta versão.
