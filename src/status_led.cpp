#include "status_led.h"

#include <Arduino.h>
#include <Ticker.h>

#include "config.h"

namespace {

Ticker startupTicker;
volatile StatusLedBase baseStatus = StatusLedBase::INITIALIZING;
volatile uint32_t baseStartedAtMs = 0;
volatile uint32_t activityEndsAtMs = 0;
volatile uint32_t activityReadyAtMs = 0;

bool before(uint32_t now, uint32_t deadline) {
  return static_cast<int32_t>(now - deadline) < 0;
}

bool baseLedOn(uint32_t now) {
  const uint32_t elapsed = now - baseStartedAtMs;
  switch (baseStatus) {
    case StatusLedBase::AP_MODE: {
      const uint32_t period = STATUS_LED_AP_ON_MS + STATUS_LED_AP_OFF_MS;
      return elapsed % period < STATUS_LED_AP_ON_MS;
    }
    case StatusLedBase::STA_CONNECTING: {
      const uint32_t firstOffAt = STATUS_LED_CONNECTING_ON_MS;
      const uint32_t secondOnAt =
          firstOffAt + STATUS_LED_CONNECTING_OFF_MS;
      const uint32_t secondOffAt =
          secondOnAt + STATUS_LED_CONNECTING_ON_MS;
      const uint32_t period = secondOffAt + STATUS_LED_CONNECTING_GAP_MS;
      const uint32_t phase = elapsed % period;
      return phase < firstOffAt ||
             (phase >= secondOnAt && phase < secondOffAt);
    }
    case StatusLedBase::WIFI_READY:
      return true;
    case StatusLedBase::INITIALIZING:
    default:
      return false;
  }
}

void writeLed(bool on) {
  digitalWrite(STATUS_LED_PIN,
               on ? STATUS_LED_ON_LEVEL : STATUS_LED_OFF_LEVEL);
}

}  // namespace

void statusLedBegin() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  writeLed(false);
  baseStatus = StatusLedBase::INITIALIZING;
  baseStartedAtMs = millis();
  activityEndsAtMs = 0;
  activityReadyAtMs = 0;

  // networkSetup() intentionally blocks while attempting STA connection.
  // A temporary timer keeps the LED state machine running during that existing
  // wait; normal appLoop() updates take over once network setup completes.
  startupTicker.attach_ms(10, statusLedUpdate);
}

void statusLedUseLoopUpdates() {
  startupTicker.detach();
  statusLedUpdate();
}

void statusLedSetBase(StatusLedBase state) {
  if (baseStatus == state) return;
  baseStatus = state;
  baseStartedAtMs = millis();
  statusLedUpdate();
}

void statusLedNotifyOscTxSuccess() {
  const uint32_t now = millis();
  if (before(now, activityEndsAtMs) || before(now, activityReadyAtMs)) return;

  activityEndsAtMs = now + STATUS_LED_ACTIVITY_OFF_MS;
  activityReadyAtMs =
      activityEndsAtMs + STATUS_LED_ACTIVITY_BASE_GAP_MS;
  statusLedUpdate();
}

void statusLedUpdate() {
  const uint32_t now = millis();
  const bool activityActive = before(now, activityEndsAtMs);
  writeLed(!activityActive && baseLedOn(now));
}
