# Hardware

**Idioma:** [Português](hardware.md) · [English](en/hardware.md)


## Placa

**ESP32-C3 Super Mini** (board PlatformIO `esp32-c3-devkitm-1`).

- USB-C para programação (CDC).
- Alimentação: bateria / 3.3 V (gestão de carga fora do escopo).

## Pinout

| Função | GPIO | Observação |
|--------|------|------------|
| Botão A | **4** | Switch para GND, pull-up; wake deep sleep |
| Botão B | **5** | Switch para GND, pull-up; wake deep sleep |
| Botão C | **3** | Switch para GND, pull-up; wake deep sleep |
| Efeito LED externo | **7** | PWM LEDC ~0→2,5 V durante MQTT (não wake) |
| LED status onboard | **8** | Azul, ativo em LOW (strapping) |
| BOOT (placa) | **9** | Strapping — **não** é GPIO0 |
| GPIO0 | — | **Não usado** pelo HAButton; no C3 **não** é pino de boot |
| Comum | GND | Compartilhado |

### GPIO0 e pinos que podem impedir o boot

No **ESP32 clássico**, o botão BOOT é GPIO0. No **ESP32-C3** isso **não se aplica**.

| Pino | Papel no C3 | Pode impedir app de subir? |
|------|-------------|----------------------------|
| **GPIO0** | IO/RTC/ADC livre | **Não** (não é strapping). HAButton não o usa. |
| **GPIO2** | Strapping | Se LOW no **reset**, boot/flash pode falhar |
| **GPIO8** | Strapping + LED | Evitar puxar LOW no reset (LED onboard ok) |
| **GPIO9** | Strapping = BOOT | Se LOW no **reset** → modo download (firmware **não** roda) |
| GPIO3/4/5 | Botões A/B/C | Não afetam modo de boot; só wake/gestos |
| GPIO7 | Efeito PWM | Não é strapping |

**Hardware:** não ligue switches em GPIO2/8/9. Não segure o botão **BOOT** da placa ao energizar pela bateria.  
No serial, o boot imprime `[boot] niveis GPIO 0=… 2=… 9=…` para conferência.

### Deep sleep (ESP32-C3)

Só **GPIO0–5** (domínio RTC) podem acordar o chip do deep sleep.  
GPIO6+ **não** são válidos como fonte de wake — o log `gpio N is an invalid deep sleep wakeup IO` indica isso.

Evitar strapping para botões: **GPIO2**, **GPIO8**, **GPIO9**.
### Esquema

```
  3V3 (pull-up interno)
        |
     [GPIO4] ---- switch A ---- GND
     [GPIO5] ---- switch B ---- GND
     [GPIO3] ---- switch C ---- GND
     [GPIO7] ---- módulo efeito LED externo
```

## Gestos e portal

Enquanto acordado, o firmware classifica:

- Clique simples / longo (A, B ou C)
- Combos curtos/longos (AB, AC, BC, ABC)

**Portal WiFiManager:** segure **A+B+C ~10 s** (LED pisca) → reinicia no AP `HAButton-XXXX`.

## Sessão acordada

1. Wake por GPIO → Wi‑Fi → MQTT.
2. Fica acordado monitorando botões.
3. Cada gesto → publica MQTT (GPIO7 já ativo desde o press) + **zera** o timer de idle.
4. Sem atividade por `sleep_delay_ms` (default **20 s**) → deep sleep.

## Pino de efeito (GPIO7)

O ESP32-C3 **não tem DAC**. O firmware usa **PWM (LEDC)** com duty equivalente a ~2,5 V (`2500/3300` de 3,3 V).

- Boot / idle / deep sleep: duty 0 (≈ 0 V).
- **Assim que** há wake por GPIO ou um botão é pressionado: GPIO7 vai de 0 → ~2,5 V **de imediato** (PWM, sem rampa).
- Após o publish MQTT: mantém 2,5 V até o envio terminar e no mínimo **0,5 s** → corta a 0 V.

Se o módulo externo precisar de tensão mais “analógica”, use filtro RC (ex.: 1 kΩ + 100 nF) entre GPIO7 e a entrada.

## Alimentação

LiPo + proteção/carga → pino **5V** + **GND** (não use 4,2 V no **3V3**).  
Em deep sleep o consumo é da ordem de µA (depende do regulador).

### Bateria sem USB

O Super Mini usa **USB CDC** como `Serial`. Sem PC conectado, `Serial.print` / `Serial.flush` antigos podiam **travar** o firmware (aparecia “ligado”, LED apagado, botões sem efeito). O firmware atual usa `Serial.setTxTimeoutMs(0)` e não faz `flush` bloqueante.

No boot a bateria: o LED onboard deve **piscar ~30 ms** — se não piscar, o chip não está executando (alimentação/brownout).

Não deixe USB do PC e bateria ligados ao mesmo tempo no Super Mini.
