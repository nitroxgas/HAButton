# Documentação HAButton

**Idioma:** [Português](README.md) · [English](en/README.md)

Projeto rápido, feito para aproveitar vaporizadores usados, para algo útil - Enviar eventos para um Home Assistant; Ele usa um pequeno microcontrolador alimentado pelo circuito do vaporizador e monitora 3 switchs mecânicos. 
Quando pressionado um ou mais botões o dispositivo acorda, conecta no wifi e publica o estado em um MQTT; 
O Home Assistant por sua vez reconhece automaticamente este dispositivo e pode ser programado para fazer automações com as informações dos eventos; 

Projeto rápido de final de semana, primeira versão, ainda muito a melhorar; 

Faça um parecido para os vaporizadores que conseguir obter, melhor forma de reaproveitar o lixo eletrônico que eles produzem.

O projeto alimenta um ESP32-C3 com os fios destinados ao sensor de pressão do vape. Opcionalmente utiliza o fio de retorno do sensor para informar o uso ao vape, caso ele tenha algum display ou led de animação na sua estrutura.

Uso os GPIOs do ESP32-C3 que monitoram interrupção para tirar o dispositivo do estado de deepsleep e executar o envio de eventos via MQTT;

Para os switchs uso 3 mecanicos padrão;

No primeiro boot o dispositivo inicializa um Access Point para receber as primeiras configurações de wifi e etc. Após conectar no WIFI e publicar os dados no MQTT de um Home Assistant, as configurações podem ser realizadas diretamente na interface. 
Lembre-se que é necessário "acordar" o dispositivo para ele receber alterações de configuração via MQTT;

Atualização via OTA simples, para ser possível o desenvolvimento e implementação usando a alimentação via vape. 

### **NUNCA LIGUE NA USB COM A ALIMENTAÇÃO PELA BATERIA DO VAPE JÁ INSTALADA, PODE DANIFICAR SEU USB**;

Você vai precisar:
 Vape usado e desmontado. **Cuidado neste processo de desmontagem, recomendo uso de luvas e óculos de proteção e descarte imediato dos componentes umidos, contém nicotina que irrita a pele e etc.** Neste caso usei um G30k-Pro, mas quase todos funcionam da mesma forma, lendo um sensor de pressão e alimentando resistências para gerar vapor;
 ESP32-C3 Super Mini;
 3 Switchs mecanicos, ou qualquer outro botão que tiver disponível;
 Fios e etc para as ligações;
 E um case, neste caso modelei um simples e imprimi em 3D; Disponível em: [MakerWorld](https://makerworld.com/en/models/3101722-habutton-g30kpro-case#profileId-3496295) ou [.3mf Neste Repositório](../DOC/pictures/habutton_G30kpro.3mf)

==Projeto feito usando Cursor e Grok-4.5 High Fast;== Sim, porque é um projeto rápido e IA é ótima para isto atualmente;

## Objetivo

Dispositivo com bateria e botões: acorda, mantém sessão (idle 20 s), publica gestos MQTT com discovery HA, OTA na LAN, efeito PWM no GPIO7 durante o envio, e volta ao deep sleep.

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