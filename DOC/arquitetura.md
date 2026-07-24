# Arquitetura

## Visão geral

O firmware executa um ciclo único em `setup()` e não permanece em `loop()`: após Wi‑Fi + MQTT, entra em deep sleep até o próximo acionamento.

```mermaid
flowchart TD
  boot[Boot ESP32-C3] --> wakeReason[Ler causa do wake]
  wakeReason --> readBtns[Ler GPIO botao A/B]
  readBtns --> wifi[WiFiManager connect ou portal]
  wifi --> mqtt[Conectar MQTT]
  mqtt --> discover[Publicar discovery HA se necessario]
  discover --> publish[Publicar acionamento]
  publish --> sleep[Wake GPIO + deep sleep]
```

## Módulos

| Arquivo | Responsabilidade |
|---------|------------------|
| `src/main.cpp` | Orquestra boot → Wi‑Fi → MQTT → sleep |
| `src/buttons.cpp` | Leitura dos pinos, janela “ambos”, deep sleep |
| `src/wifi_setup.cpp` | WiFiManager, parâmetros customizados, NVS |
| `src/mqtt_ha.cpp` | PubSubClient, discovery MQTT, pulso ON/OFF |
| `include/config.h` | Pinos, timeouts e defaults |

## Wake e botões

- Causa `ESP_SLEEP_WAKEUP_GPIO`: leitura imediata de GPIO4/GPIO5 (ativo em nível baixo).
- Janela de ~80 ms (`BUTTON_BOTH_WINDOW_MS`) para detectar o segundo botão em acionamento próximo.
- Antes de dormir: espera soltar os botões (até ~3 s) para evitar wake em laço.

## Persistência (NVS)

Namespace `habutton`:

- Credenciais/parâmetros MQTT e nomes (`mqtt_host`, `mqtt_port`, …).
- Flag `disc_done`: evita republicar discovery retained a cada wake (reseta se a config MQTT mudar).

As credenciais de Wi‑Fi ficam a cargo do WiFiManager (armazenamento próprio).

## MQTT / Home Assistant

- Discovery retained em `{mqtt_prefix}/binary_sensor/{deviceId}_{btn}/config`.
- Estado em `{device_name}/{mac}/{btn}/state` com pulso `ON` → `OFF`.
- Sem tópico de availability: o deep sleep não deve marcar a entidade como *unavailable* entre presses.
- `expire_after` / `off_delay` no discovery ajudam o HA a tratar o evento como momentâneo.

## Energia

- Após publicar: `WiFi.disconnect` + `WIFI_OFF` + `esp_deep_sleep_start()`.
- Wake configurado com máscara GPIO4|GPIO5, nível baixo (`ESP_GPIO_WAKEUP_GPIO_LOW`).
- Em falha de Wi‑Fi após timeout do portal (~90 s): dorme para preservar bateria.
