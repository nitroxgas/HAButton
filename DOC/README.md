# Documentação HAButton

**Idioma:** [Português](README.md) · [English](en/README.md)

| Documento | Conteúdo |
|-----------|----------|
| [arquitetura.md](arquitetura.md) | Sessão acordada, módulos, MQTT |
| [hardware.md](hardware.md) | Pinout A/B/C, efeito GPIO7, portal |
| [configuracao.md](configuracao.md) | Portal, sleep 20s, event_types |
| [mqtt-topicos.md](mqtt-topicos.md) | Todos os tópicos MQTT (runtime + discovery) |
| [mqtt-config.md](mqtt-config.md) | Debug logs + config remota via MQTT/HA |
| [homeassistant-contadores.md](homeassistant-contadores.md) | Contadores/automações: UI ou packages YAML |
| [homeassistant-fallback-yaml.md](homeassistant-fallback-yaml.md) | YAML se discovery falhar |
| [desenvolvimento.md](desenvolvimento.md) | Build USB e overview |
| [ota.md](ota.md) | Guia completo de update OTA |

## Objetivo

Device a bateria com 3 botões: acorda, mantém sessão (idle 20 s), publica gestos MQTT com discovery HA, OTA na LAN, efeito PWM no GPIO7 durante o envio, e volta ao deep sleep.
