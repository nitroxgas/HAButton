# Desenvolvimento

## Requisitos

- [PlatformIO Core](https://platformio.org/) (`pio` no PATH)
- Cabo USB-C e drivers (Windows: normalmente WinUSB/CDC automático no C3)
- Git (e GitHub CLI para o repositório remoto)

## Estrutura do repositório

```
HAButton/
├── platformio.ini
├── include/           # config.h, headers públicos
├── src/               # main, buttons, wifi_setup, mqtt_ha
├── DOC/               # esta documentação
├── README.md
└── .gitignore
```

## Build e upload

```bash
cd HAButton
pio run                 # compila
pio run -t upload       # grava no ESP32-C3
pio device monitor      # serial 115200 (USB CDC)
```

Environment: `esp32-c3-supermini` em `platformio.ini`.

Flags importantes:

- `ARDUINO_USB_MODE=1`
- `ARDUINO_USB_CDC_ON_BOOT=1` — Serial via USB no Super Mini.

## Bibliotecas

Declaradas em `platformio.ini` (`lib_deps`):

- `tzapu/WiFiManager`
- `knolleary/PubSubClient`
- `bblanchon/ArduinoJson`

## Logs úteis no monitor

Prefixos:

- `[buttons]` — wake e estado A/B
- `[wifi]` — conexão / AP
- `[mqtt]` — connect, discovery, publishes
- `[sleep]` — entrada em deep sleep

## Troubleshooting

| Sintoma | Verificação |
|---------|-------------|
| Não aparece Serial | Conferir CDC, cabo dados, `pio device list` |
| Loop de wake | Botão preso em GND; firmware espera soltar antes de dormir |
| Portal não abre | Aguardar falha de Wi‑Fi; timeout 90 s |
| HA não cria entidades | Discovery MQTT ligado; prefixo `homeassistant`; broker acessível do ESP |
| Publica mas HA não reage | Assinar o state topic com um cliente MQTT; checar user/pass |
| Build antigo / cache | `pio run -t clean` depois `pio run` |

## Erase completo

```bash
pio run -t erase
pio run -t upload
```

Apaga NVS (Wi‑Fi + MQTT) e força novo portal.
