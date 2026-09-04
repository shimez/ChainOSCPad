#pragma once
#include <Arduino.h>
#include "config.h"

constexpr uint8_t MAX_OSC_MESSAGES = 8;
enum OscValueType : uint8_t { OSC_TYPE_FLOAT=0, OSC_TYPE_INT=1, OSC_TYPE_STRING=2 };
enum InputMode : uint8_t { INPUT_MODE_PRESS_RELEASE=0, INPUT_MODE_SEQUENCE=1 };

struct OscMessageSetting { String address; String value; OscValueType type=OSC_TYPE_FLOAT; };
struct SequenceSetting { String address="/sequence"; OscValueType type=OSC_TYPE_FLOAT; float start=0, end=10, step=1, current=0; };
struct ButtonInputSetting {
  InputMode mode=INPUT_MODE_PRESS_RELEASE;
  OscMessageSetting pressMessages[MAX_OSC_MESSAGES];
  OscMessageSetting releaseMessages[MAX_OSC_MESSAGES];
  uint8_t pressMessageCount=1, releaseMessageCount=1;
  SequenceSetting sequence;
};
struct KeyInputSetting { String displayName; ButtonInputSetting button; };
struct EncoderInputSetting {
  String displayName="Encoder";
  String rotationAddress="/avatar/parameters/Encoder";
  bool sendIncrement=false;
  bool wrapAround=true;
  float absoluteInputMin=0, absoluteInputMax=20, incrementScale=.05f;
  float outputMin=0, outputMax=1;
  OscValueType outputType=OSC_TYPE_FLOAT;
  ButtonInputSetting click;
};

void inputSettingsSetup();
KeyInputSetting& inputKeySetting(uint8_t index);
EncoderInputSetting& inputEncoderSetting();
bool inputSettingsSaveKey(uint8_t index,const KeyInputSetting& setting);
bool inputSettingsSaveEncoder(const EncoderInputSetting& setting);
bool inputSettingsResetKey(uint8_t index);
bool inputSettingsResetEncoder();
bool inputSettingsReset();
bool inputOscAddressValid(const String& address);
bool inputOscMessageValid(const OscMessageSetting& message);
bool inputSequenceSettingValid(const SequenceSetting& sequence);
bool inputButtonSettingValid(const ButtonInputSetting& setting);
bool inputEncoderSettingValid(const EncoderInputSetting& encoder);
bool inputParseFloat(const String& text,float& value);
bool inputParseInt(const String& text,int32_t& value);
