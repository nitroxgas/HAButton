# Hardware

## Placa

**ESP32-C3 Super Mini** (compatível com board PlatformIO `esp32-c3-devkitm-1`).

- USB-C para programação (CDC habilitado no firmware).
- Alimentação em operação: bateria (típico 3.3 V regulado / LiPo com módulo de carga externo — fora do escopo do firmware).

## Pinout dos botões

| Função | GPIO | Observação |
|--------|------|------------|
| Botão A | **4** | Switch mecânico para GND |
| Botão B | **5** | Switch mecânico para GND |
| LED status | **8** | Azul onboard, **ativo em LOW** |
| Comum | GND | Compartilhado |

Constantes em `include/config.h` (`BTN_A_PIN`, `BTN_B_PIN`, `STATUS_LED_PIN`).

O LED acende após Wi‑Fi conectar (durante MQTT/discovery) e apaga antes do deep sleep.

### Esquema lógico

```
  3V3 (pull-up interno no GPIO)
        |
     [GPIO4] ---- switch A ---- GND
     [GPIO5] ---- switch B ---- GND
```

- Modo: `INPUT_PULLUP`.
- Pressionado = nível baixo → wake de deep sleep.

## Alimentação por bateria

Sugestão típica (não incluída neste repositório):

- Célula LiPo + módulo de carga/proteção (ex.: TP4056 com proteção) + regulador 3.3 V se necessário.
- Conectar VOUT regulado a `3V3` e GND comum.
- **Não** alimentar só por USB e bateria em conflito sem diodo/PMIC adequado.

Consumo: em deep sleep o C3 fica na ordem de µA (depende do regulador e periféricos externos). O Wi‑Fi ativo no wake é o maior custo — por isso o ciclo é curto.

## Montagem mecânica

Dois switches de teclado (MX ou equivalente) com fios curtos aos GPIOs/GND. Debounce é tratado por leitura + espera de soltura antes do sleep; se necessário, adicione capacitor 100 nF no pino para GND.
