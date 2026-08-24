#include <Arduino.h>
#include <ArduinoOSCWiFi.h>
#include <WiFi.h>

#include "config.h"
#include "app.h"

namespace {

struct DebouncedInput {
  bool stable = false;
  bool candidate = false;
  uint32_t candidateSinceMs = 0;
};

DebouncedInput keys[KEY_COUNT];
DebouncedInput encoderButton;

uint32_t lastMatrixScanUs = 0;
uint32_t lastWifiAttemptMs = 0;

uint8_t encoderPreviousState = 0;
int8_t encoderTransitionAccumulator = 0;
int32_t encoderAbsoluteValue = ENCODER_ABSOLUTE_MIN;

bool wifiConfigured() {
  return strcmp(WIFI_SSID, "YOUR_WIFI_SSID") != 0 && WIFI_SSID[0] != '\0';
}

void beginWifi() {
  if (!wifiConfigured()) {
    Serial.println("[WiFi] Edit include/config.h before uploading.");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiAttemptMs = millis();
  Serial.printf("[WiFi] Connecting to %s\n", WIFI_SSID);
}

void maintainWifi() {
  static wl_status_t previousStatus = WL_NO_SHIELD;
  const wl_status_t status = WiFi.status();

  if (status != previousStatus) {
    if (status == WL_CONNECTED) {
      Serial.printf("[WiFi] Connected, IP=%s, OSC target=%s:%u\n",
                    WiFi.localIP().toString().c_str(), OSC_TARGET_HOST,
                    OSC_TARGET_PORT);
    } else {
      Serial.printf("[WiFi] Status=%d\n", static_cast<int>(status));
    }
    previousStatus = status;
  }

  if (!wifiConfigured() || status == WL_CONNECTED) return;
  const uint32_t now = millis();
  if (now - lastWifiAttemptMs >= WIFI_RETRY_INTERVAL_MS) {
    lastWifiAttemptMs = now;
    Serial.println("[WiFi] Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void sendFloat(const char* address, float value) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[OSC] skipped (WiFi disconnected): %s %.3f\n", address,
                  value);
    return;
  }
  OscWiFi.send(OSC_TARGET_HOST, OSC_TARGET_PORT, address, value);
  Serial.printf("[OSC] %s %.3f -> %s:%u\n", address, value, OSC_TARGET_HOST,
                OSC_TARGET_PORT);
}

void sendKey(uint8_t keyIndex, bool pressed) {
  char address[32];
  snprintf(address, sizeof(address), "/chainoscpad/key/%u", keyIndex + 1);
  sendFloat(address, pressed ? 1.0f : 0.0f);
}

bool updateDebounce(DebouncedInput& input, bool sample, uint32_t debounceMs,
                    uint32_t nowMs) {
  if (sample != input.candidate) {
    input.candidate = sample;
    input.candidateSinceMs = nowMs;
  }
  if (input.stable != input.candidate &&
      nowMs - input.candidateSinceMs >= debounceMs) {
    input.stable = input.candidate;
    return true;
  }
  return false;
}

void scanMatrix() {
  const uint32_t nowUs = micros();
  if (nowUs - lastMatrixScanUs < MATRIX_SCAN_INTERVAL_US) return;
  lastMatrixScanUs = nowUs;

  bool samples[KEY_COUNT] = {};

  // Inactive rows remain high impedance. The active row is driven LOW and
  // columns use internal pull-ups, so a pressed switch reads LOW.
  for (uint8_t row = 0; row < ROW_COUNT; ++row) {
    pinMode(ROW_PINS[row], OUTPUT);
    digitalWrite(ROW_PINS[row], LOW);
    delayMicroseconds(3);

    for (uint8_t col = 0; col < COL_COUNT; ++col) {
      samples[row * COL_COUNT + col] = digitalRead(COL_PINS[col]) == LOW;
    }

    pinMode(ROW_PINS[row], INPUT);
  }

  const uint32_t nowMs = millis();
  for (uint8_t key = 0; key < KEY_COUNT; ++key) {
    if (updateDebounce(keys[key], samples[key], KEY_DEBOUNCE_MS, nowMs)) {
      Serial.printf("[Key] %u %s\n", key + 1,
                    keys[key].stable ? "pressed" : "released");
      sendKey(key, keys[key].stable);
    }
  }
}

void sendEncoderAbsolute() {
  const int32_t span = ENCODER_ABSOLUTE_MAX - ENCODER_ABSOLUTE_MIN;
  const float mapped = static_cast<float>(encoderAbsoluteValue -
                                          ENCODER_ABSOLUTE_MIN) /
                       static_cast<float>(span);
  sendFloat(ENCODER_OSC_ADDRESS, mapped);
  Serial.printf("[Encoder] absolute=%ld mapped=%.3f\n",
                static_cast<long>(encoderAbsoluteValue), mapped);
}

void applyEncoderDetent(int8_t direction) {
  const int32_t span = ENCODER_ABSOLUTE_MAX - ENCODER_ABSOLUTE_MIN;
  encoderAbsoluteValue += direction;
  while (encoderAbsoluteValue >= ENCODER_ABSOLUTE_MAX)
    encoderAbsoluteValue -= span;
  while (encoderAbsoluteValue < ENCODER_ABSOLUTE_MIN)
    encoderAbsoluteValue += span;
  sendEncoderAbsolute();
}

void pollEncoder() {
  // Gray-code transition table. Invalid/bouncing two-bit jumps contribute 0.
  static constexpr int8_t TRANSITION_TABLE[16] = {
      0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

  const uint8_t current =
      (digitalRead(ENCODER_A_PIN) == HIGH ? 2 : 0) |
      (digitalRead(ENCODER_B_PIN) == HIGH ? 1 : 0);
  const uint8_t transition = (encoderPreviousState << 2) | current;
  encoderPreviousState = current;
  encoderTransitionAccumulator += TRANSITION_TABLE[transition];

  if (encoderTransitionAccumulator >= ENCODER_TRANSITIONS_PER_DETENT) {
    encoderTransitionAccumulator = 0;
    applyEncoderDetent(1);
  } else if (encoderTransitionAccumulator <= -ENCODER_TRANSITIONS_PER_DETENT) {
    encoderTransitionAccumulator = 0;
    applyEncoderDetent(-1);
  }

  const bool pressed = digitalRead(ENCODER_BUTTON_PIN) == LOW;
  if (updateDebounce(encoderButton, pressed, ENCODER_BUTTON_DEBOUNCE_MS,
                     millis())) {
    sendFloat(ENCODER_BUTTON_OSC_ADDRESS,
              encoderButton.stable ? 1.0f : 0.0f);
    Serial.printf("[Encoder button] %s\n",
                  encoderButton.stable ? "pressed" : "released");
  }
}

void setupPins() {
  for (uint8_t row = 0; row < ROW_COUNT; ++row) pinMode(ROW_PINS[row], INPUT);
  for (uint8_t col = 0; col < COL_COUNT; ++col)
    pinMode(COL_PINS[col], INPUT_PULLUP);

  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  encoderPreviousState =
      (digitalRead(ENCODER_A_PIN) == HIGH ? 2 : 0) |
      (digitalRead(ENCODER_B_PIN) == HIGH ? 1 : 0);
}

}  // namespace

void appSetup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);
  Serial.println("\nChainOSCPad fixed-config hardware test");
  setupPins();
  beginWifi();
}

void appLoop() {
  maintainWifi();
  scanMatrix();
  pollEncoder();
  delay(1);
}

#if defined(CHAINOSCPAD_PLATFORMIO)
void setup() { appSetup(); }
void loop() { appLoop(); }
#endif
