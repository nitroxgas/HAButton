# OTA update (Over-The-Air)

**Language:** [English](ota.md) · [Português](../ota.md)

HAButton uses **ArduinoOTA** on the local Wi‑Fi network. The device only accepts uploads while **awake** (session after a click). In deep sleep OTA does not respond.

## Prerequisites

1. Firmware already flashed via USB at least once (dual-OTA partitions: `min_spiffs.csv`).
2. PC and ESP32 on the **same LAN** (same SSID / VLAN).
3. Device **awake**:
   - Press a button to wake.
   - Default idle: **20 s** with no new clicks → deep sleep.
   - Each new gesture **restarts** the timer.
   - While an OTA upload is in progress, the idle timer is **automatically restarted** until OTA finishes (ESP will not sleep mid-transfer).
4. Know the **IP** or **hostname** and the **OTA password**.

### Hostname and password

| Item | Value |
|------|--------|
| Hostname | `{device_name}-{mac4}{mac5}` lowercase hex — e.g. device `habutton`, MAC `…:91:C0` → `habutton-91c0` |
| mDNS | `habutton-91c0.local` (if Windows resolves mDNS) |
| OTA password | Portal **OTA Password** (default `habutton-ota`) |

On Serial Monitor, after Wi‑Fi connects:

```text
[ota] pronto hostname=habutton-91c0
[wifi] conectado SSID=... IP=192.168.1.103 ...
```

Use that **IP** if `.local` does not resolve.

### Extend the OTA window

In the WiFiManager portal (A+B+C ~10 s):

- **Sleep delay ms (idle / OTA)** — increase (e.g. `60000` = 1 min) so you have more time without clicking during upload.

---

## Method 1 — PlatformIO (recommended)

`pio run` does **not** accept `--upload-protocol` or `--upload-flags` (only `--upload-port`).  
OTA protocol is in the **`esp32-c3-ota`** env in `platformio.ini`.

### Steps

1. Open the project (e.g. `d:\Coding\HAButton`).
2. Wake the ESP (press a button) and note the IP from Serial (or your router).
3. On the PC, run **immediately** (still within the idle window):

```bash
cd d:\Coding\HAButton
pio run -e esp32-c3-ota -t upload --upload-port 192.168.1.103
```

Replace the IP. Default env password is `habutton-ota`.

Hostname alternative (if mDNS works):

```bash
pio run -e esp32-c3-ota -t upload --upload-port habutton-91c0.local
```

4. On ESP Serial you should see `[ota] start`, progress, and `[ota] end`. The device reboots with the new firmware.

### Different OTA password

Edit `platformio.ini` (env `esp32-c3-ota`):

```ini
upload_flags =
    --auth=your-password
```

### USB upload (default)

Env `esp32-c3-supermini` still uses USB/`esptool`:

```bash
pio run -e esp32-c3-supermini -t upload --upload-port COM4
```

---

## Method 2 — Python `espota.py` (PlatformIO)

With firmware already built:

```bash
pio run
python %USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\espota.py -i 192.168.1.103 -a habutton-ota -f .pio\build\esp32-c3-supermini\firmware.bin
```

(On Linux/macOS, `espota.py` lives under `~/.platformio/packages/...`.)

---

## Practical checklist

1. [ ] Build: `pio run`
2. [ ] Increase sleep delay in the portal if upload is slow (optional)
3. [ ] Press a button on HAButton (wake)
4. [ ] Confirm on Serial: `[ota] pronto hostname=...` and IP
5. [ ] Send OTA: `pio run -e esp32-c3-ota -t upload --upload-port <IP>`
6. [ ] Wait for reboot; validate version / behavior

Tip: keep a finger on the button and click again if idle is about to expire **before** starting the upload (do not click mid-transfer).

---

## Troubleshooting

| Symptom | Likely cause | What to do |
|---------|--------------|------------|
| `No such option: --upload-protocol` | Invalid flag on `pio run` | Use `-e esp32-c3-ota` (Method 1) |
| `No response` / timeout | Device sleeping | Wake with button; increase sleep delay; retry upload quickly |
| `Authentication Failed` | Wrong password | Check portal and `upload_flags` in ini |
| Host does not resolve (`.local`) | mDNS on Windows | Use numeric **IP** |
| PC and ESP on different networks | Guest Wi‑Fi / VLAN | Same SSID / same subnet |
| USB upload OK, OTA not | Partition without OTA | Flash USB with this project (`min_spiffs.csv`) |
| OTA starts then fails mid-way | Idle expired / weak Wi‑Fi | Larger sleep delay; move closer to AP |

ArduinoOTA error codes on Serial (`[ota] error N`):

| Code | Meaning |
|------|---------|
| 0 | OTA_AUTH_ERROR — password |
| 1 | OTA_BEGIN_ERROR |
| 2 | OTA_CONNECT_ERROR |
| 3 | OTA_RECEIVE_ERROR |
| 4 | OTA_END_ERROR |

---

## First flash (USB)

OTA does **not** replace the first flash. Use a USB-C cable:

```bash
pio run -e esp32-c3-supermini -t upload --upload-port COMx
```

After that, updates can be OTA-only.

## Security

- Change the default OTA password in the portal on shared networks.
- OTA only on LAN; do not expose the port to the internet.
- Password is stored in ESP NVS (same namespace as Wi‑Fi/MQTT config).
