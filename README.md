# HAButton

Firmware PlatformIO para **ESP32-C3 Super Mini** com dois switches de teclado mecânico, deep sleep por GPIO, WiFiManager e publicação MQTT com descoberta automática no Home Assistant.

## Início rápido

```bash
pio run
pio run -t upload
pio device monitor
```

No primeiro boot (ou se o Wi‑Fi falhar), conecte-se ao AP `HAButton-XXXX` e configure Wi‑Fi + MQTT.

Documentação completa: [DOC/README.md](DOC/README.md).

## Hardware (resumo)

| Função   | Pino   |
|----------|--------|
| Botão A  | GPIO4  |
| Botão B  | GPIO5  |
| Comum    | GND    |

Switches para GND, pull-up interno. Alimentação por bateria (detalhes em [DOC/hardware.md](DOC/hardware.md)).

## Licença

Uso pessoal / projeto próprio — ajuste conforme necessário.
