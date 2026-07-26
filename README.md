# HAButton

Firmware PlatformIO para **ESP32-C3 Super Mini**: 3 switches, sessão acordada, gestos MQTT (Home Assistant), OTA e pino de efeito LED.

## Início rápido

```bash
pio run
pio run -t upload
pio device monitor
```

- Portal: segure **A+B+C ~10 s** → AP `HAButton-XXXX`
- Idle default: **20 s** (reinicia a cada gesto)
- OTA: [DOC/ota.md](DOC/ota.md)
- Docs: [DOC/README.md](DOC/README.md)

## Hardware (resumo)

| Função | GPIO |
|--------|------|
| Botões A/B/C | 4 / 5 / 6 |
| Efeito LED | 7 (PWM ~2 V) |
| LED status | 8 |

Versão firmware: **1.3.0**
