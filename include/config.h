#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Fixed test configuration
// ---------------------------------------------------------------------------
// Copy secrets.example.h to secrets.h and edit it. secrets.h is ignored by
// Git, preventing Wi-Fi credentials from being published accidentally.
#if __has_include("secrets.h")
#include "secrets.h"
#else
inline constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
inline constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
inline constexpr char OSC_TARGET_HOST[] = "192.168.1.100";
inline constexpr uint16_t OSC_TARGET_PORT = 9000;
#endif

inline constexpr char APP_NAME[] = "ChainOSCPad";
inline constexpr char APP_VERSION[] = "0.6.1";

#if defined(CHAINOSCPAD_BOARD_XIAO_ESP32S3)
inline constexpr char HARDWARE_NAME[] = "XIAO ESP32S3";
#elif defined(CHAINOSCPAD_BOARD_XIAO_ESP32C6)
inline constexpr char HARDWARE_NAME[] = "XIAO ESP32C6";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
inline constexpr char HARDWARE_NAME[] = "XIAO ESP32S3";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
inline constexpr char HARDWARE_NAME[] = "XIAO ESP32C6";
#else
inline constexpr char HARDWARE_NAME[] = "Unknown ESP32";
#endif
inline constexpr char WIFI_AP_SSID[] = "ChainOSCPad-Setup";
inline constexpr char WIFI_AP_PASSWORD[] = "12345678";
inline constexpr char WIFI_MDNS_HOST[] = "chainoscpad";
inline constexpr char PREFS_NAMESPACE[] = "chainoscpad";
inline constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
inline constexpr uint32_t NETWORK_RESTART_DELAY_MS = 1200;

// Use XIAO logical pin names so the same source supports both ESP32S3 and
// ESP32C6. Each Arduino board definition maps D0..D10 to its own GPIOs.
inline constexpr uint8_t ROW_PINS[] = {D0, D1, D2, D3};
inline constexpr uint8_t COL_PINS[] = {D4, D5, D6};
inline constexpr uint8_t ENCODER_A_PIN = D7;
inline constexpr uint8_t ENCODER_B_PIN = D8;
inline constexpr uint8_t ENCODER_BUTTON_PIN = D9;

inline constexpr uint8_t ROW_COUNT = 4;
inline constexpr uint8_t COL_COUNT = 3;
inline constexpr uint8_t KEY_COUNT = ROW_COUNT * COL_COUNT;

inline constexpr uint32_t SERIAL_BAUD = 115200;
inline constexpr uint32_t MATRIX_SCAN_INTERVAL_US = 1000;
inline constexpr uint32_t KEY_DEBOUNCE_MS = 20;
inline constexpr uint32_t ENCODER_BUTTON_DEBOUNCE_MS = 20;

// Most mechanical encoders generate four valid quadrature transitions per
// detent. Change to 2 or 1 if one click needs multiple physical detents.
inline constexpr int8_t ENCODER_TRANSITIONS_PER_DETENT = 4;
