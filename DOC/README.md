# Documentação HAButton

Índice do planejamento e operação do projeto.

| Documento | Conteúdo |
|-----------|----------|
| [arquitetura.md](arquitetura.md) | Fluxo de boot, módulos e deep sleep |
| [hardware.md](hardware.md) | Pinout, ligações dos botões e bateria |
| [configuracao.md](configuracao.md) | Portal WiFiManager, MQTT e Home Assistant |
| [desenvolvimento.md](desenvolvimento.md) | Build PlatformIO, estrutura e troubleshooting |

## Objetivo

Dispositivo a bateria que:

1. Dorme em deep sleep.
2. Acorda por interrupção GPIO ao pressionar um ou dois switches mecânicos.
3. Conecta ao Wi‑Fi (portal configurável no primeiro acesso / falha).
4. Publica o acionamento via MQTT com discovery para o Home Assistant.
5. Volta ao deep sleep.
