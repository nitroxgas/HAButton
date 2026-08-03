# Development

**Language:** [English](development.md) · [Português](../desenvolvimento.md)

## USB build / upload

```bash
pio run
pio run -t upload
pio device monitor
```

Environment: `esp32-c3-supermini`. Partitions: `min_spiffs.csv` (dual OTA).

## OTA (local network)

Detailed steps (walkthrough, PlatformIO, troubleshooting): **[ota.md](ota.md)**.

Summary: wake the device → same LAN →  
`pio run -e esp32-c3-ota -t upload --upload-port <IP>`

## Firmware

Version: `FW_VERSION` in `include/config.h` (current **1.4.5**).

## Troubleshooting

| Symptom | Action |
|---------|--------|
| HA has no device | ACL `homeassistant/#`; Serial `VERIFY` |
| OTA timeout | Device awake? Same network? Correct password/IP? |
| Weak/odd effect LED | PWM ~2.5 V; RC filter if you need analog |
| Portal | Hold A+B+C 10 s |
