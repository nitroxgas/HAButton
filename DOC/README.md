# Documentação HAButton

| Documento | Conteúdo |
|-----------|----------|
| [arquitetura.md](arquitetura.md) | Sessão acordada, módulos, MQTT |
| [hardware.md](hardware.md) | Pinout A/B/C, efeito GPIO7, portal |
| [configuracao.md](configuracao.md) | Portal, sleep 20s, event_types |
| [homeassistant-contadores.md](homeassistant-contadores.md) | Contagem por event_type |
| [homeassistant-fallback-yaml.md](homeassistant-fallback-yaml.md) | YAML se discovery falhar |
| [desenvolvimento.md](desenvolvimento.md) | Build USB e overview |
| [ota.md](ota.md) | Guia completo de update OTA |

## Objetivo

Device a bateria com 3 botões: acorda, mantém sessão (idle 20 s), publica gestos MQTT com discovery HA, OTA na LAN, efeito PWM no GPIO7 durante o envio, e volta ao deep sleep.
