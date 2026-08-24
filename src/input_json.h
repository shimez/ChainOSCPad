#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "input_settings.h"

constexpr char CHAINOSCPAD_SETTINGS_FORMAT[] = "ChainOSCPad-settings";
constexpr char CHAINOSC_PRESET_FORMAT[] = "ChainOSC-device-preset";
constexpr char CHAINOSC_LEGACY_PRESET_FORMAT[] = "M5ChainOSC-device-preset";
constexpr int INPUT_JSON_SCHEMA_VERSION = 1;
constexpr int CHAIN_KEY_DEVICE_TYPE = 3;
constexpr int CHAIN_ENCODER_DEVICE_TYPE = 1;

String inputJsonQuote(const String& value);
String inputKeyJson(const KeyInputSetting& setting, uint8_t index, bool includeIdentity);
String inputEncoderJson(const EncoderInputSetting& setting, bool includeIdentity);
bool inputKeyFromJson(JsonObjectConst object, KeyInputSetting& setting,
                      bool includeIdentity, uint8_t expectedIndex, String& error);
bool inputEncoderFromJson(JsonObjectConst object, EncoderInputSetting& setting,
                          bool includeIdentity, String& error);
