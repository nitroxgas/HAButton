#pragma once

// Defaults locais opcionais (gitignored). Deve vir ANTES dos #ifndef abaixo.
#if __has_include("secrets.h")
#include "secrets.h"
#endif

// ---------------------------------------------------------------------------
// Hardware — ESP32-C3 Super Mini
// Switches mecanicos para GND, INPUT_PULLUP, wake em nivel baixo.
// ---------------------------------------------------------------------------
#ifndef BTN_A_PIN
#define BTN_A_PIN 4
#endif

#ifndef BTN_B_PIN
#define BTN_B_PIN 5
#endif

// Janela (ms) para detectar ambos os botoes apos o wake
#ifndef BUTTON_BOTH_WINDOW_MS
#define BUTTON_BOTH_WINDOW_MS 80
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
// MQTT / Home Assistant — defaults do primeiro boot (editaveis no portal)
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

#ifndef MQTT_CONNECT_TIMEOUT_MS
#define MQTT_CONNECT_TIMEOUT_MS 8000
#endif

#ifndef MQTT_POST_PUBLISH_MS
#define MQTT_POST_PUBLISH_MS 800
#endif

// Namespace Preferences (NVS)
#ifndef NVS_NAMESPACE
#define NVS_NAMESPACE "habutton"
#endif
