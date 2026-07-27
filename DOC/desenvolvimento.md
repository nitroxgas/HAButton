# Desenvolvimento

**Idioma:** [Português](desenvolvimento.md) · [English](en/development.md)

## Build / upload USB

```bash
pio run
pio run -t upload
pio device monitor
```

Environment: `esp32-c3-supermini`. Partições: `min_spiffs.csv` (dual OTA).

## OTA (rede local)

Instruções detalhadas (passo a passo, PlatformIO, troubleshooting): **[ota.md](ota.md)**.

Resumo: acorde o device → mesma LAN →  
`pio run -e esp32-c3-ota -t upload --upload-port <IP>`

## Firmware

Versão: `FW_VERSION` em `include/config.h` (atual **1.4.2**).

## Troubleshooting

| Sintoma | Ação |
|---------|------|
| HA sem device | ACL `homeassistant/#`; Serial `VERIFY` |
| OTA timeout | Device acordado? Mesma rede? Senha/IP corretos? |
| Efeito LED fraco/estranho | PWM ~2,5 V; filtro RC se precisar analógico |
| Portal | Segurar A+B+C 10 s |
