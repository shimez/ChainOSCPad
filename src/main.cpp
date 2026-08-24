#include <Arduino.h>
#include <ArduinoOSCWiFi.h>
#include <math.h>

#include "config.h"
#include "app.h"
#include "input_settings.h"
#include "network_manager.h"

namespace {

struct DebouncedInput {
  bool stable = false;
  bool candidate = false;
  uint32_t candidateSinceMs = 0;
};

DebouncedInput keys[KEY_COUNT];
DebouncedInput encoderButton;

uint32_t lastMatrixScanUs = 0;
uint8_t encoderPreviousState = 0;
int8_t encoderTransitionAccumulator = 0;
float encoderAbsoluteValue = 0.0f;

bool oscReady(const String& address, const String& value) {
  if (!networkIsConnected()) {
    Serial.printf("[OSC] skipped (WiFi disconnected): %s %s\n",
                  address.c_str(), value.c_str());
    return false;
  }
  return true;
}

void logSent(const String& address, const String& value,
             OscValueType type) {
  Serial.printf("[OSC] %s %s type=%u -> %s:%u\n", address.c_str(),
                value.c_str(), static_cast<unsigned>(type),
                networkOscHost().c_str(), networkOscPort());
}

void sendConfiguredMessage(const OscMessageSetting& message) {
  if (!oscReady(message.address, message.value)) return;
  if (message.type == OSC_TYPE_FLOAT) {
    float value = 0;
    if (!inputParseFloat(message.value, value)) return;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(),
                 message.address.c_str(), value);
  } else if (message.type == OSC_TYPE_INT) {
    int32_t value = 0;
    if (!inputParseInt(message.value, value)) return;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(),
                 message.address.c_str(), static_cast<int>(value));
  } else {
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(),
                 message.address.c_str(), message.value.c_str());
  }
  logSent(message.address, message.value, message.type);
}

void sendMappedValue(const String& address, float mapped,
                     OscValueType type) {
  String text;
  if (type == OSC_TYPE_INT) {
    const int value = static_cast<int>(lroundf(mapped));
    text = String(value);
    if (!oscReady(address, text)) return;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(), address.c_str(),
                 value);
  } else if (type == OSC_TYPE_STRING) {
    text = String(mapped, 3);
    if (!oscReady(address, text)) return;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(), address.c_str(),
                 text.c_str());
  } else {
    text = String(mapped, 3);
    if (!oscReady(address, text)) return;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(), address.c_str(),
                 mapped);
  }
  logSent(address, text, type);
}

void sendButton(ButtonInputSetting& setting, bool pressed) {
  if (setting.mode == INPUT_MODE_SEQUENCE) {
    if (!pressed) return;
    SequenceSetting& sequence = setting.sequence;
    sendMappedValue(sequence.address, sequence.current, sequence.type);
    float next = sequence.current + sequence.step;
    if ((sequence.step >= 0 && next > sequence.end + 1e-6f) ||
        (sequence.step < 0 && next < sequence.end - 1e-6f)) {
      next = sequence.start;
    }
    sequence.current = next;
    return;
  }
  OscMessageSetting* messages = pressed ? setting.pressMessages
                                        : setting.releaseMessages;
  const uint8_t count = pressed ? setting.pressMessageCount
                                : setting.releaseMessageCount;
  for (uint8_t i = 0; i < count; ++i) sendConfiguredMessage(messages[i]);
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
      KeyInputSetting& setting = inputKeySetting(key);
      sendButton(setting.button, keys[key].stable);
    }
  }
}

void applyEncoderDetent(int8_t direction) {
  const EncoderInputSetting& setting = inputEncoderSetting();
  float mapped = 0.0f;
  if (setting.sendIncrement) {
    mapped = static_cast<float>(direction) * setting.incrementScale;
    mapped = constrain(mapped, min(setting.outputMin, setting.outputMax),
                       max(setting.outputMin, setting.outputMax));
  } else {
    const float span = setting.absoluteInputMax - setting.absoluteInputMin;
    encoderAbsoluteValue += direction;
    while (encoderAbsoluteValue >= setting.absoluteInputMax)
      encoderAbsoluteValue -= span;
    while (encoderAbsoluteValue < setting.absoluteInputMin)
      encoderAbsoluteValue += span;
    const float ratio = (encoderAbsoluteValue - setting.absoluteInputMin) / span;
    mapped = setting.outputMin + ratio * (setting.outputMax - setting.outputMin);
  }
  sendMappedValue(setting.rotationAddress, mapped, setting.outputType);
  Serial.printf("[Encoder] position=%.3f mapped=%.3f mode=%s\n",
                encoderAbsoluteValue, mapped,
                setting.sendIncrement ? "increment" : "absolute");
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
    EncoderInputSetting& setting = inputEncoderSetting();
    sendButton(setting.click, encoderButton.stable);
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
  Serial.printf("\n%s v%s\n", APP_NAME, APP_VERSION);
  setupPins();
  inputSettingsSetup();
  encoderAbsoluteValue = inputEncoderSetting().absoluteInputMin;
  networkSetup();
}

void appLoop() {
  networkLoop();
  scanMatrix();
  pollEncoder();
  delay(1);
}

#if defined(CHAINOSCPAD_PLATFORMIO)
void setup() { appSetup(); }
void loop() { appLoop(); }
#endif
