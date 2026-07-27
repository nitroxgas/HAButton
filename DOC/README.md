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

Projeto rápido, feito para aproveitar vaporizadores usados, para algo útil - Enviar eventos para um Home Assistant; Ele usa um pequeno microcontrolador alimentado pelo circuito do vaporizador e monitora 3 switchs mecânicos. Quando pressionado um ou mais botões o dispositivo acorda, conecta no wifi e publica o estado em um MQTT; O Home Assistant por sua vez reconhece automaticamente este dispositivo e pode ser programado para fazer automações com as informações dos eventos; 
Projeto rápido de final de semana, primeira versão, ainda muito a melhorar; 
Faça um parecido para os vaporizadores que conseguir obter, melhor forma de reaproveitar o lixo eletrônico que eles produzem.

## Objetivo

Device a bateria com 3 botões: acorda, mantém sessão (idle 20 s), publica gestos MQTT com discovery HA, OTA na LAN, efeito PWM no GPIO7 durante o envio, e volta ao deep sleep.
