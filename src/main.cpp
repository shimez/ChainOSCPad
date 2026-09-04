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

#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
struct EncoderDiagnosticEvent {
  uint32_t timeUs;
  uint32_t pollGapUs;
  uint8_t previous;
  uint8_t current;
  int8_t delta;
  int8_t accumulator;
};

constexpr size_t ENCODER_DIAGNOSTIC_EVENT_CAPACITY = 96;
EncoderDiagnosticEvent
    encoderDiagnosticEvents[ENCODER_DIAGNOSTIC_EVENT_CAPACITY];
size_t encoderDiagnosticEventCount = 0;
uint32_t encoderDiagnosticDroppedEvents = 0;
uint32_t encoderDiagnosticLastPollUs = 0;
uint32_t encoderDiagnosticLastChangeUs = 0;
uint32_t encoderDiagnosticTransitions = 0;
uint32_t encoderDiagnosticInvalidTransitions = 0;
uint32_t encoderDiagnosticMaxPollGapUs = 0;
uint32_t encoderDiagnosticMaxNetworkUs = 0;
uint32_t encoderDiagnosticMaxMatrixUs = 0;
uint32_t encoderDiagnosticMaxEncoderUs = 0;

void flushEncoderDiagnostics() {
  if (encoderDiagnosticEventCount == 0) return;
  for (size_t i = 0; i < encoderDiagnosticEventCount; ++i) {
    const EncoderDiagnosticEvent& event = encoderDiagnosticEvents[i];
    Serial.printf(
        "[Encoder diag] t=%luus AB=%u%u->%u%u delta=%d acc=%d gap=%luus\n",
        static_cast<unsigned long>(event.timeUs),
        (event.previous >> 1) & 1, event.previous & 1,
        (event.current >> 1) & 1, event.current & 1,
        static_cast<int>(event.delta), static_cast<int>(event.accumulator),
        static_cast<unsigned long>(event.pollGapUs));
  }
  Serial.printf(
      "[Encoder diag summary] changes=%lu invalid=%lu dropped=%lu "
      "max-gap=%luus network=%luus matrix=%luus encoder=%luus\n",
      static_cast<unsigned long>(encoderDiagnosticTransitions),
      static_cast<unsigned long>(encoderDiagnosticInvalidTransitions),
      static_cast<unsigned long>(encoderDiagnosticDroppedEvents),
      static_cast<unsigned long>(encoderDiagnosticMaxPollGapUs),
      static_cast<unsigned long>(encoderDiagnosticMaxNetworkUs),
      static_cast<unsigned long>(encoderDiagnosticMaxMatrixUs),
      static_cast<unsigned long>(encoderDiagnosticMaxEncoderUs));
  encoderDiagnosticEventCount = 0;
  encoderDiagnosticDroppedEvents = 0;
  encoderDiagnosticTransitions = 0;
  encoderDiagnosticInvalidTransitions = 0;
  encoderDiagnosticMaxPollGapUs = 0;
  encoderDiagnosticMaxNetworkUs = 0;
  encoderDiagnosticMaxMatrixUs = 0;
  encoderDiagnosticMaxEncoderUs = 0;
  // Exclude the diagnostic output time itself from the next poll-gap result.
  encoderDiagnosticLastPollUs = micros();
}
#endif

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

bool sendMappedValue(const String& address, float mapped,
                     OscValueType type) {
  String text;
  if (type == OSC_TYPE_INT) {
    const int value = static_cast<int>(lroundf(mapped));
    text = String(value);
    if (!oscReady(address, text)) return false;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(), address.c_str(),
                 value);
  } else if (type == OSC_TYPE_STRING) {
    text = String(mapped, 3);
    if (!oscReady(address, text)) return false;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(), address.c_str(),
                 text.c_str());
  } else {
    text = String(mapped, 3);
    if (!oscReady(address, text)) return false;
    OscWiFi.send(networkOscHost().c_str(), networkOscPort(), address.c_str(),
                 mapped);
  }
  logSent(address, text, type);
  return true;
}

void sendButton(ButtonInputSetting& setting, bool pressed) {
  if (setting.mode == INPUT_MODE_SEQUENCE) {
    if (!pressed) return;
    SequenceSetting& sequence = setting.sequence;
    if (!sendMappedValue(sequence.address, sequence.current, sequence.type)) {
      return;
    }
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

  for (uint8_t row = 0; row < ROW_COUNT; ++row) {
    digitalWrite(ROW_PINS[row], LOW);
    delayMicroseconds(3);

    for (uint8_t col = 0; col < COL_COUNT; ++col) {
      samples[row * COL_COUNT + col] = digitalRead(COL_PINS[col]) == LOW;
    }

    digitalWrite(ROW_PINS[row], HIGH);
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
    if(setting.wrapAround){
      while (encoderAbsoluteValue >= setting.absoluteInputMax) encoderAbsoluteValue -= span;
      while (encoderAbsoluteValue < setting.absoluteInputMin) encoderAbsoluteValue += span;
    }else{
      encoderAbsoluteValue=constrain(encoderAbsoluteValue,setting.absoluteInputMin,setting.absoluteInputMax);
    }
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

#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  const uint32_t pollUs = micros();
  const uint32_t pollGapUs = encoderDiagnosticLastPollUs == 0
                                 ? 0
                                 : pollUs - encoderDiagnosticLastPollUs;
  encoderDiagnosticLastPollUs = pollUs;
  encoderDiagnosticMaxPollGapUs =
      max(encoderDiagnosticMaxPollGapUs, pollGapUs);
#endif

  const uint8_t previous = encoderPreviousState;
  const uint8_t current =
      (digitalRead(ENCODER_A_PIN) == HIGH ? 2 : 0) |
      (digitalRead(ENCODER_B_PIN) == HIGH ? 1 : 0);
  const uint8_t transition = (previous << 2) | current;
  encoderPreviousState = current;
  const int8_t delta = TRANSITION_TABLE[transition];
  encoderTransitionAccumulator += delta;

#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  if (current != previous) {
    ++encoderDiagnosticTransitions;
    if (delta == 0) ++encoderDiagnosticInvalidTransitions;
    encoderDiagnosticLastChangeUs = pollUs;
    if (encoderDiagnosticEventCount < ENCODER_DIAGNOSTIC_EVENT_CAPACITY) {
      encoderDiagnosticEvents[encoderDiagnosticEventCount++] = {
          pollUs, pollGapUs, previous, current, delta,
          encoderTransitionAccumulator};
    } else {
      ++encoderDiagnosticDroppedEvents;
    }
  }
  if (encoderDiagnosticEventCount > 0 &&
      pollUs - encoderDiagnosticLastChangeUs >= 200000)
    flushEncoderDiagnostics();
#endif

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
  for (uint8_t row = 0; row < ROW_COUNT; ++row) {
    pinMode(ROW_PINS[row], OUTPUT_OPEN_DRAIN);
    digitalWrite(ROW_PINS[row], HIGH);
  }
  for (uint8_t col = 0; col < COL_COUNT; ++col)
    pinMode(COL_PINS[col], INPUT_PULLUP);

  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  encoderPreviousState =
      (digitalRead(ENCODER_A_PIN) == HIGH ? 2 : 0) |
      (digitalRead(ENCODER_B_PIN) == HIGH ? 1 : 0);
#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  Serial.printf("[Encoder diag] enabled A=D7/GPIO%u B=D8/GPIO%u initial=%u%u "
                "transitions-per-detent=%d\n",
                static_cast<unsigned>(ENCODER_A_PIN),
                static_cast<unsigned>(ENCODER_B_PIN),
                (encoderPreviousState >> 1) & 1, encoderPreviousState & 1,
                static_cast<int>(ENCODER_TRANSITIONS_PER_DETENT));
#endif
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
#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  uint32_t phaseStartUs = micros();
#endif
  networkLoop();
#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  uint32_t phaseEndUs = micros();
  encoderDiagnosticMaxNetworkUs =
      max(encoderDiagnosticMaxNetworkUs, phaseEndUs - phaseStartUs);
  phaseStartUs = phaseEndUs;
#endif
  scanMatrix();
#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  phaseEndUs = micros();
  encoderDiagnosticMaxMatrixUs =
      max(encoderDiagnosticMaxMatrixUs, phaseEndUs - phaseStartUs);
  phaseStartUs = phaseEndUs;
#endif
  pollEncoder();
#if defined(CHAINOSCPAD_ENCODER_DIAGNOSTICS)
  phaseEndUs = micros();
  encoderDiagnosticMaxEncoderUs =
      max(encoderDiagnosticMaxEncoderUs, phaseEndUs - phaseStartUs);
#endif
  // Do not call delay(1) or yield() here. On the current ESP32C5
  // Arduino/FreeRTOS configuration both can wait for an approximately 50 ms
  // scheduler tick, which is long enough to miss quadrature transitions.
  // Higher-priority Wi-Fi/system tasks are still scheduled preemptively.
}

#if defined(CHAINOSCPAD_PLATFORMIO)
void setup() { appSetup(); }
void loop() { appLoop(); }
#endif
