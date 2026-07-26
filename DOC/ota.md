# Atualização OTA (Over-The-Air)

O HAButton usa **ArduinoOTA** na rede Wi‑Fi local. O device só aceita upload enquanto está **acordado** (sessão após um clique). Em deep sleep o OTA não responde.

## Pré-requisitos

1. Firmware já gravado via USB pelo menos uma vez (partições dual-OTA: `min_spiffs.csv`).
2. PC e ESP32 na **mesma LAN** (mesmo SSID / VLAN).
3. Device **acordado**:
   - Pressione um botão para acordar.
   - Idle default: **20 s** sem novos cliques → deep sleep.
   - Cada novo gesto **reinicia** o timer.
   - Se um upload OTA estiver em andamento, o timer de idle é **reiniciado automaticamente** até o OTA terminar (o ESP não dorme no meio do processo).
4. Conhecer **IP** ou **hostname** e a **senha OTA**.

### Hostname e senha

| Item | Valor |
|------|--------|
| Hostname | `{device_name}-{mac4}{mac5}` em hex minúsculo — ex.: device `habutton`, MAC `…:91:C0` → `habutton-91c0` |
| mDNS | `habutton-91c0.local` (se o Windows resolver mDNS) |
| Senha OTA | Portal **OTA Password** (default `habutton-ota`) |

No Serial Monitor, ao conectar Wi‑Fi, aparece algo como:

```text
[ota] pronto hostname=habutton-91c0
[wifi] conectado SSID=... IP=192.168.1.103 ...
```

Use esse **IP** se `.local` não resolver.

### Estender a janela OTA

No portal WiFiManager (A+B+C ~10 s):

- **Sleep delay ms (idle / OTA)** — aumente (ex. `60000` = 1 min) para ter mais tempo sem precisar clicar durante o upload.

---

## Método 1 — PlatformIO (recomendado)

### Passo a passo

1. Abra o projeto em `d:\Coding\HAButton`.
2. Acorde o ESP (pressione um botão) e anote o IP no Serial (ou no roteador).
3. No PC, rode **imediatamente** (ainda na janela idle):

```bash
cd d:\Coding\HAButton
pio run -t upload --upload-protocol espota --upload-port 192.168.1.103 --upload-flags "--auth=habutton-ota"
```

Substitua o IP e a senha pelos seus.

Alternativa com hostname (se mDNS funcionar):

```bash
pio run -t upload --upload-protocol espota --upload-port habutton-91c0.local --upload-flags "--auth=habutton-ota"
```

4. No Serial do ESP deve aparecer `[ota] start`, progresso e `[ota] end`. O device reinicia com o novo firmware.

### Configurar uma vez no `platformio.ini`

Para não repetir flags, adicione (ajuste IP/senha):

```ini
[env:esp32-c3-supermini]
; ... demais opções existentes ...
upload_protocol = espota
upload_port = 192.168.1.103
upload_flags =
    --auth=habutton-ota
```

Depois, com o device acordado:

```bash
pio run -t upload
```

> Para voltar ao upload USB, comente/remova `upload_protocol = espota` ou use  
> `pio run -t upload --upload-protocol esptool --upload-port COM4`.

---

## Método 2 — Python `espota.py` (PlatformIO)

Com o firmware já compilado:

```bash
pio run
python %USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py -i 192.168.1.103 -a habutton-ota -f .pio\build\esp32-c3-supermini\firmware.bin
```

(No Linux/macOS o caminho do `espota.py` fica sob `~/.platformio/packages/...`.)

---

## Sequência prática (checklist)

1. [ ] Compilar: `pio run`
2. [ ] Aumentar sleep delay no portal se o upload for lento (opcional)
3. [ ] Pressionar botão no HAButton (acordar)
4. [ ] Confirmar no Serial: `[ota] pronto hostname=...` e IP
5. [ ] Enviar OTA pelo PlatformIO / espota
6. [ ] Aguardar reboot; validar versão / comportamento

Dica: mantenha um dedo no botão e clique de novo se o idle estiver perto de expirar **antes** de iniciar o upload (não clique no meio do transfer).

---

## Troubleshooting

| Sintoma | Causa provável | O que fazer |
|---------|----------------|-------------|
| `No response` / timeout | Device dormindo | Acordar com botão; aumentar sleep delay; repetir upload rápido |
| `Authentication Failed` | Senha errada | Conferir **OTA Password** no portal (default `habutton-ota`) |
| Host não resolve (`.local`) | mDNS no Windows | Usar o **IP** numérico |
| PC e ESP em redes diferentes | Wi‑Fi guest / VLAN | Mesmo SSID / mesma subnet |
| Upload USB ok, OTA não | Partição sem OTA | Gravar USB com este projeto (`min_spiffs.csv`) |
| OTA inicia e falha no meio | Idle expirou / Wi‑Fi fraco | Sleep delay maior; aproximar do AP; não dormir no meio |

Códigos de erro ArduinoOTA no Serial (`[ota] error N`):

| Código | Significado |
|--------|-------------|
| 0 | OTA_AUTH_ERROR — senha |
| 1 | OTA_BEGIN_ERROR |
| 2 | OTA_CONNECT_ERROR |
| 3 | OTA_RECEIVE_ERROR |
| 4 | OTA_END_ERROR |

---

## Primeira gravação (USB)

OTA **não** substitui a primeira flash. Use cabo USB-C:

```bash
pio run -t upload --upload-protocol esptool --upload-port COMx
```

Depois disso, as atualizações podem ser só por OTA.

## Segurança

- Troque a senha OTA padrão no portal em redes compartilhadas.
- OTA só na LAN; não exponha a porta na internet.
- A senha fica na NVS do ESP (mesmo namespace da config Wi‑Fi/MQTT).
