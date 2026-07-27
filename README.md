# HAButton

PlatformIO firmware for **ESP32-C3 Super Mini**: 3 switches, awake session, MQTT gestures (Home Assistant), OTA, and LED effect pin.

**Docs:** [English](DOC/en/README.md) · [Português](DOC/README.md)

## Quick start

```bash
pio run
pio run -t upload
pio device monitor
```

- Portal: hold **A+B+C ~10 s** → AP `HAButton-XXXX`
- Default idle: **20 s** (restarts on each gesture)
- OTA: [English](DOC/en/ota.md) · [Português](DOC/ota.md)
- Full documentation: [DOC/en](DOC/en/README.md) · [DOC (PT)](DOC/README.md)

## Hardware (summary)

| Function | GPIO |
|----------|------|
| Buttons A/B/C | 4 / 5 / 3 |
| Effect LED | 7 (PWM ~2.5 V) |
| Status LED | 8 |

Firmware version: **1.4.2** (`include/config.h`)

---

## Português

Firmware PlatformIO para **ESP32-C3 Super Mini**: 3 switches, sessão acordada, gestos MQTT (Home Assistant), OTA e pino de efeito LED.

Documentação completa: [DOC/README.md](DOC/README.md).
