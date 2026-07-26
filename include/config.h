#pragma once

#if __has_include("secrets.h")
#include "secrets.h"
#endif

// ---------------------------------------------------------------------------
// Hardware — ESP32-C3 Super Mini
// ---------------------------------------------------------------------------
#ifndef BTN_A_PIN
#define BTN_A_PIN 4
#endif

#ifndef BTN_B_PIN
#define BTN_B_PIN 5
#endif

// Deep-sleep wake no C3: somente GPIO0–5 (RTC). GPIO6+ não acordam.
#ifndef BTN_C_PIN
#define BTN_C_PIN 3
#endif

#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 8
#endif

#ifndef STATUS_LED_ACTIVE_LOW
#define STATUS_LED_ACTIVE_LOW 1
#endif

// Pino de efeito externo (PWM ~0→2,5 V durante MQTT).
#ifndef EFFECT_PIN
#define EFFECT_PIN 7
#endif

#ifndef EFFECT_PWM_CHANNEL
#define EFFECT_PWM_CHANNEL 0
#endif

#ifndef EFFECT_PWM_FREQ_HZ
#define EFFECT_PWM_FREQ_HZ 5000
#endif

#ifndef EFFECT_PWM_RES_BITS
#define EFFECT_PWM_RES_BITS 10
#endif

#ifndef EFFECT_TARGET_MV
#define EFFECT_TARGET_MV 2500
#endif

#ifndef EFFECT_SUPPLY_MV
#define EFFECT_SUPPLY_MV 3300
#endif

// Tempo mínimo em 2,5 V após ligar o efeito (além de esperar o publish MQTT).
#ifndef EFFECT_HOLD_MIN_MS
#define EFFECT_HOLD_MIN_MS 500
#endif

// Gestos
#ifndef LONG_PRESS_MS
#define LONG_PRESS_MS 800
#endif

#ifndef CONFIG_CHORD_HOLD_MS
#define CONFIG_CHORD_HOLD_MS 10000
#endif

#ifndef BUTTON_CHORD_WINDOW_MS
#define BUTTON_CHORD_WINDOW_MS 100
#endif

#ifndef WIFI_PORTAL_FORCED_TIMEOUT_S
#define WIFI_PORTAL_FORCED_TIMEOUT_S 180
#endif

// ---------------------------------------------------------------------------
// WiFiManager
// ---------------------------------------------------------------------------
#ifndef WM_AP_NAME_PREFIX
#define WM_AP_NAME_PREFIX "HAButton"
#endif

#ifndef WIFI_CONNECT_TIMEOUT_S
#define WIFI_CONNECT_TIMEOUT_S 50
#endif

#ifndef WIFI_PORTAL_TIMEOUT_S
#define WIFI_PORTAL_TIMEOUT_S 90
#endif

// ---------------------------------------------------------------------------
// MQTT / Home Assistant — defaults
// ---------------------------------------------------------------------------
#ifndef DEFAULT_MQTT_HOST
#define DEFAULT_MQTT_HOST "homeassistant.local"
#endif

#ifndef DEFAULT_MQTT_PORT
#define DEFAULT_MQTT_PORT "1883"
#endif

#ifndef DEFAULT_MQTT_USER
#define DEFAULT_MQTT_USER ""
#endif

#ifndef DEFAULT_MQTT_PASS
#define DEFAULT_MQTT_PASS ""
#endif

#ifndef DEFAULT_MQTT_PREFIX
#define DEFAULT_MQTT_PREFIX "homeassistant"
#endif

#ifndef DEFAULT_DEVICE_NAME
#define DEFAULT_DEVICE_NAME "habutton"
#endif

#ifndef DEFAULT_BTN_A_NAME
#define DEFAULT_BTN_A_NAME "Botao A"
#endif

#ifndef DEFAULT_BTN_B_NAME
#define DEFAULT_BTN_B_NAME "Botao B"
#endif

#ifndef DEFAULT_BTN_C_NAME
#define DEFAULT_BTN_C_NAME "Botao C"
#endif

#ifndef DEFAULT_SLEEP_DELAY_MS
#define DEFAULT_SLEEP_DELAY_MS "20000"
#endif

#ifndef SLEEP_DELAY_MIN_MS
#define SLEEP_DELAY_MIN_MS 1000
#endif

#ifndef SLEEP_DELAY_MAX_MS
#define SLEEP_DELAY_MAX_MS 300000
#endif

#ifndef DEFAULT_OTA_PASS
#define DEFAULT_OTA_PASS "habutton-ota"
#endif

#ifndef MQTT_CONNECT_TIMEOUT_MS
#define MQTT_CONNECT_TIMEOUT_MS 8000
#endif

#ifndef NVS_NAMESPACE
#define NVS_NAMESPACE "habutton"
#endif

#ifndef FW_VERSION
#define FW_VERSION "1.3.2"
#endif

// Incrementar ao mudar o schema de discovery MQTT (força limpeza + republicação).
#ifndef DISCOVERY_SCHEMA_VERSION
#define DISCOVERY_SCHEMA_VERSION 3
#endif
