#pragma once

enum class StatusLedBase {
  INITIALIZING,
  AP_MODE,
  STA_CONNECTING,
  WIFI_READY,
};

void statusLedBegin();
void statusLedUseLoopUpdates();
void statusLedSetBase(StatusLedBase state);
void statusLedNotifyOscTxSuccess();
void statusLedUpdate();
