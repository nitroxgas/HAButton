# Hardware

**Language:** [English](hardware.md) · [Português](../hardware.md)

## Board

**ESP32-C3 Super Mini** (PlatformIO board `esp32-c3-devkitm-1`).

- USB-C for programming (CDC).
- Power: battery / 3.3 V (charge management out of scope).

## Pinout

| Function | GPIO | Notes |
|----------|------|-------|
| Button A | **4** | Switch to GND, pull-up; deep-sleep wake |
| Button B | **5** | Switch to GND, pull-up; deep-sleep wake |
| Button C | **3** | Switch to GND, pull-up; deep-sleep wake |
| External effect LED | **7** | LEDC PWM ~0→2.5 V during MQTT (not wake) |
| Onboard status LED | **8** | Blue, active LOW (strapping) |
| BOOT (board) | **9** | Strapping — **not** GPIO0 |
| GPIO0 | — | **Unused** by HAButton; on C3 it is **not** the boot pin |
| Common | GND | Shared |

### GPIO0 and pins that can block boot

On classic **ESP32**, BOOT is GPIO0. On **ESP32-C3** that does **not** apply.

| Pin | Role on C3 | Can prevent app boot? |
|-----|------------|------------------------|
| **GPIO0** | Free IO/RTC/ADC | **No** (not strapping). HAButton does not use it. |
| **GPIO2** | Strapping | If LOW at **reset**, boot/flash may fail |
| **GPIO8** | Strapping + LED | Avoid pulling LOW at reset (onboard LED is fine) |
| **GPIO9** | Strapping = BOOT | If LOW at **reset** → download mode (firmware **does not** run) |
| GPIO3/4/5 | Buttons A/B/C | Do not affect boot mode; only wake/gestures |
| GPIO7 | Effect PWM | Not strapping |

**Hardware:** do not wire switches to GPIO2/8/9. Do not hold the board **BOOT** button when powering from battery.  
On serial, boot prints `[boot] niveis GPIO 0=… 2=… 9=…` for verification.

### Deep sleep (ESP32-C3)

Only **GPIO0–5** (RTC domain) can wake the chip from deep sleep.  
GPIO6+ are **not** valid wake sources — the log `gpio N is an invalid deep sleep wakeup IO` means that.

Avoid strapping pins for buttons: **GPIO2**, **GPIO8**, **GPIO9**.

### Schematic

```
  3V3 (internal pull-up)
        |
     [GPIO4] ---- switch A ---- GND
     [GPIO5] ---- switch B ---- GND
     [GPIO3] ---- switch C ---- GND
     [GPIO7] ---- external effect LED module
```

## Gestures and portal

While awake, firmware classifies:

- Short / long press (A, B, or C)
- Short / long combos (AB, AC, BC, ABC)

**WiFiManager portal:** hold **A+B+C ~10 s** (LED blinks) → restarts on AP `HAButton-XXXX`.

## Awake session

1. GPIO wake → Wi‑Fi → MQTT.
2. Stays awake polling buttons.
3. Each gesture (including wake) → identify → GPIO7 ON → MQTT → GPIO7 OFF → **reset** idle.
4. No activity for `sleep_delay_ms` (default **20 s**) → deep sleep.

## Effect pin (GPIO7)

ESP32-C3 has **no DAC**. Firmware uses **PWM (LEDC)** with duty ≈ 2.5 V (`2500/3300` of 3.3 V).

- Boot / idle / deep sleep: duty 0 (≈ 0 V).
- On GPIO wake or button press: GPIO7 goes 0 → ~2.5 V **immediately** (PWM, no ramp).
- After MQTT publish: holds 2.5 V until send completes and at least **0.5 s** → cuts to 0 V.

If the external module needs a smoother analog voltage, add an RC filter (e.g. 1 kΩ + 100 nF) between GPIO7 and the input.

## Power

LiPo + protection/charger → **5V** pin + **GND** (do not feed 4.2 V into **3V3**).  
In deep sleep, current is on the order of µA (depends on the regulator).

### Battery without USB

Super Mini uses **USB CDC** as `Serial`. Without a PC, older `Serial.print` / `Serial.flush` could **hang** the firmware (looked “on”, LED off, buttons dead). Current firmware uses `Serial.setTxTimeoutMs(0)` and avoids blocking flush.

On battery boot: onboard LED should **blink ~30 ms** — if it does not, the chip is not running (power/brownout).

Do not leave PC USB and battery connected at the same time on the Super Mini.
