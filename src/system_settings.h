#pragma once

#include <Arduino.h>

bool systemSettingsSetup();

const String& systemSettingsWifiSsid();
const String& systemSettingsWifiPassword();
const String& systemSettingsOscHost();
uint16_t systemSettingsOscPort();
bool systemSettingsLanguageConfigured();
uint8_t systemSettingsLanguage();

bool systemSettingsSaveWifi(const String& ssid, const String& password);
bool systemSettingsClearWifi();
bool systemSettingsSaveOsc(const String& host, uint16_t port);
bool systemSettingsSaveLanguage(uint8_t language);
bool systemSettingsClearAll();
