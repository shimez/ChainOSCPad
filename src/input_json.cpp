#include "input_json.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

namespace {
bool fail(String& error, const char* code) {
  error = code;
  return false;
}

bool requiredFields(JsonObjectConst object, const char* const* fields,
                    size_t count, String& error) {
  for (size_t index = 0; index < count; ++index)
    if (!object.containsKey(fields[index]))
      return fail(error, "E_PRESET_REQUIRED_FIELD_MISSING");
  return true;
}

bool validPresetAddress(String address, String& error) {
  address.trim();
  if (address.length() > 192) return fail(error, "E_OSC_ADDRESS_TOO_LONG");
  if (address.isEmpty() || address[0] != '/')
    return fail(error, "E_OSC_ADDRESS_INVALID");
  for (size_t index = 0; index < address.length(); ++index) {
    const char c = address[index];
    if (isWhitespace(c) || c == '#' || c == '*' || c == ',' || c == '?' ||
        c == '[' || c == ']' || c == '{' || c == '}')
      return fail(error, "E_OSC_ADDRESS_INVALID");
  }
  return true;
}

bool validateMessage(JsonObjectConst object, String& error) {
  if (object.isNull()) return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  static const char* const required[] = {"address", "value", "type"};
  if (!requiredFields(object, required, 3, error)) return false;
  if (!object["address"].is<const char*>() ||
      !object["value"].is<const char*>() || !object["type"].is<int>())
    return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  if (!validPresetAddress(object["address"].as<const char*>(), error))
    return false;
  const String value = object["value"].as<const char*>();
  if (value.length() > 128) return fail(error, "E_OSC_VALUE_TOO_LONG");
  const int type = object["type"].as<int>();
  if (type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING)
    return fail(error, "E_OSC_TYPE_INVALID");
  if (type == OSC_TYPE_FLOAT) {
    errno = 0;
    char* end = nullptr;
    const float parsed = strtof(value.c_str(), &end);
    if (!end || end == value.c_str() || *end != '\0' || errno == ERANGE ||
        !isfinite(parsed))
      return fail(error, "E_OSC_FLOAT32_INVALID");
  } else if (type == OSC_TYPE_INT) {
    errno = 0;
    char* end = nullptr;
    const long parsed = strtol(value.c_str(), &end, 10);
    if (!end || end == value.c_str() || *end != '\0' || errno == ERANGE ||
        parsed < INT32_MIN || parsed > INT32_MAX)
      return fail(error, "E_OSC_INT32_INVALID");
  }
  return true;
}

bool validateMessages(JsonVariantConst pressValue,
                      JsonVariantConst releaseValue, String& error) {
  if (!pressValue.is<JsonArrayConst>() || !releaseValue.is<JsonArrayConst>())
    return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  const JsonArrayConst press = pressValue.as<JsonArrayConst>();
  const JsonArrayConst release = releaseValue.as<JsonArrayConst>();
  if (press.size() + release.size() > MAX_OSC_MESSAGES)
    return fail(error, "E_OSC_MESSAGE_COUNT_EXCEEDED");
  for (JsonVariantConst item : press) {
    if (!item.is<JsonObjectConst>())
      return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    if (!validateMessage(item.as<JsonObjectConst>(), error)) return false;
  }
  for (JsonVariantConst item : release) {
    if (!item.is<JsonObjectConst>())
      return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    if (!validateMessage(item.as<JsonObjectConst>(), error)) return false;
  }
  return true;
}

bool validateSequence(JsonObjectConst object, bool legacy, String& error) {
  if (object.isNull()) return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  static const char* const required[] = {"address", "type", "start", "end",
                                         "step"};
  for (const char* field : required)
    if (!object.containsKey(field))
      return fail(error, "E_SEQUENCE_REQUIRED_FIELD_MISSING");
  if (!object["address"].is<const char*>() || !object["type"].is<int>())
    return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  if (!object["start"].is<float>() || !object["end"].is<float>() ||
      !object["step"].is<float>())
    return fail(error, "E_SEQUENCE_VALUE_INVALID");
  if (!validPresetAddress(object["address"].as<const char*>(), error))
    return false;
  const int type = object["type"].as<int>();
  if ((type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING) && !legacy)
    return fail(error, "E_OSC_TYPE_INVALID");
  const float start = object["start"].as<float>();
  const float end = object["end"].as<float>();
  const float step = object["step"].as<float>();
  if (!isfinite(start) || !isfinite(end) || !isfinite(step))
    return fail(error, "E_SEQUENCE_VALUE_INVALID");
  if (step == 0.0f) return fail(error, "E_SEQUENCE_STEP_ZERO");
  if ((start < end && step < 0.0f) || (start > end && step > 0.0f))
    return fail(error, "E_SEQUENCE_DIRECTION_INVALID");
  return true;
}

bool validateRange(JsonObjectConst range, bool legacy, String& error) {
  (void)legacy;
  if (range.isNull()) return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  static const char* const required[] = {"outMin", "outMax", "type"};
  if (!requiredFields(range, required, 3, error)) return false;
  if (!range["outMin"].is<float>() || !range["outMax"].is<float>() ||
      !range["type"].is<int>())
    return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  const int type = range["type"].as<int>();
  if (type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING)
    return fail(error, "E_OSC_TYPE_INVALID");
  if (!isfinite(range["outMin"].as<float>()) ||
      !isfinite(range["outMax"].as<float>()))
    return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
  return true;
}

bool onlyFields(JsonObjectConst object, const char* const* fields,
                size_t count) {
  for (JsonPairConst pair : object) {
    bool allowed = false;
    for (size_t index = 0; index < count; ++index)
      if (strcmp(pair.key().c_str(), fields[index]) == 0) {
        allowed = true;
        break;
      }
    if (!allowed) return false;
  }
  return true;
}

bool validAmountOutput(float outputMin, float outputMax, int outputType) {
  if (!isfinite(outputMin) || !isfinite(outputMax) ||
      !(outputMin < outputMax))
    return false;
  const float span = outputMax - outputMin;
  if (!isfinite(span)) return false;
  if (outputType != OSC_TYPE_INT) return true;
  const double roundedMin = round(static_cast<double>(outputMin));
  const double roundedMax = round(static_cast<double>(outputMax));
  return roundedMin >= static_cast<double>(INT32_MIN) &&
         roundedMax <= static_cast<double>(INT32_MAX);
}

bool validateEncoderV2(JsonObjectConst encoder, String& error) {
  static const char* const common[] = {"rotationAddress", "rotationMode",
      "outputType", "pushMode", "press", "release", "sequence"};
  if (!requiredFields(encoder, common, 7, error)) return false;
  if (!encoder["rotationAddress"].is<const char*>() ||
      !encoder["rotationMode"].is<const char*>() ||
      !encoder["outputType"].is<int>() || !encoder["pushMode"].is<int>() ||
      !encoder["press"].is<JsonArrayConst>() ||
      !encoder["release"].is<JsonArrayConst>() ||
      !encoder["sequence"].is<JsonObjectConst>())
    return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
  if (!validPresetAddress(encoder["rotationAddress"].as<const char*>(), error))
    return false;
  const int outputType = encoder["outputType"].as<int>();
  if (outputType < OSC_TYPE_FLOAT || outputType > OSC_TYPE_STRING)
    return fail(error, "E_OSC_TYPE_INVALID");
  const int pushMode = encoder["pushMode"].as<int>();
  if (pushMode < INPUT_MODE_PRESS_RELEASE || pushMode > INPUT_MODE_SEQUENCE)
    return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");

  const String mode = encoder["rotationMode"].as<const char*>();
  if (mode == "amount") {
    static const char* const fields[] = {"rotationAddress", "rotationMode",
        "rangeSteps", "wrap", "clockwiseIncreases", "outputMin", "outputMax", "outputType",
        "pushMode", "press", "release", "sequence"};
    static const char* const required[] = {"rangeSteps", "wrap",
                                           "clockwiseIncreases", "outputMin",
                                           "outputMax"};
    if (!requiredFields(encoder, required, 5, error)) return false;
    if (!onlyFields(encoder, fields, 12))
      return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
    if (!encoder["rangeSteps"].is<float>() || !encoder["wrap"].is<bool>() ||
        !encoder["clockwiseIncreases"].is<bool>() ||
        !encoder["outputMin"].is<float>() ||
        !encoder["outputMax"].is<float>())
      return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    const float rangeSteps = encoder["rangeSteps"].as<float>();
    if (!isfinite(rangeSteps) || floorf(rangeSteps) != rangeSteps ||
        rangeSteps < 1 || rangeSteps > 65535 ||
        !validAmountOutput(encoder["outputMin"].as<float>(),
                           encoder["outputMax"].as<float>(), outputType))
      return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
  } else if (mode == "direction") {
    static const char* const fields[] = {"rotationAddress", "rotationMode",
        "clockwiseValue", "counterClockwiseValue", "outputType", "pushMode",
        "press", "release", "sequence"};
    static const char* const required[] = {"clockwiseValue",
                                           "counterClockwiseValue"};
    if (!requiredFields(encoder, required, 2, error)) return false;
    if (!onlyFields(encoder, fields, 9))
      return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
    if (outputType == OSC_TYPE_STRING) {
      if (!encoder["clockwiseValue"].is<const char*>() ||
          !encoder["counterClockwiseValue"].is<const char*>())
        return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
      if (String(encoder["clockwiseValue"].as<const char*>()).length() > 128 ||
          String(encoder["counterClockwiseValue"].as<const char*>()).length() >
              128)
        return fail(error, "E_OSC_VALUE_TOO_LONG");
    } else if (outputType == OSC_TYPE_INT) {
      if (!encoder["clockwiseValue"].is<int32_t>() ||
          !encoder["counterClockwiseValue"].is<int32_t>())
        return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    } else {
      if (!encoder["clockwiseValue"].is<float>() ||
          !encoder["counterClockwiseValue"].is<float>())
        return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
      if (!isfinite(encoder["clockwiseValue"].as<float>()) ||
          !isfinite(encoder["counterClockwiseValue"].as<float>()))
        return fail(error, "E_OSC_FLOAT32_INVALID");
    }
  } else {
    return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
  }
  return validateMessages(encoder["press"], encoder["release"], error) &&
         validateSequence(encoder["sequence"].as<JsonObjectConst>(), false,
                          error);
}

String messageJson(const OscMessageSetting& message) {
  return String("{\"address\":") + inputJsonQuote(message.address) +
         ",\"value\":" + inputJsonQuote(message.value) +
         ",\"type\":" + String(static_cast<int>(message.type)) + "}";
}

String messageArrayJson(const OscMessageSetting* messages, uint8_t count) {
  String output = "[";
  for (uint8_t index = 0; index < count; ++index) {
    if (index) output += ',';
    output += messageJson(messages[index]);
  }
  output += ']';
  return output;
}

String sequenceJson(const SequenceSetting& sequence) {
  return String("{\"address\":") + inputJsonQuote(sequence.address) +
         ",\"type\":" + String(static_cast<int>(sequence.type)) +
         ",\"start\":" + String(sequence.start, 6) +
         ",\"end\":" + String(sequence.end, 6) +
         ",\"step\":" + String(sequence.step, 6) + "}";
}

String buttonJson(const ButtonInputSetting& button) {
  return String("{\"mode\":") + String(static_cast<int>(button.mode)) +
         ",\"press\":" +
         messageArrayJson(button.pressMessages, button.pressMessageCount) +
         ",\"release\":" +
         messageArrayJson(button.releaseMessages, button.releaseMessageCount) +
         ",\"sequence\":" + sequenceJson(button.sequence) + "}";
}

bool jsonMessage(JsonObjectConst object, OscMessageSetting& message,
                 String& error) {
  if (object.isNull() || !object["address"].is<const char*>() ||
      !object["value"].is<const char*>() || !object["type"].is<int>()) {
    error = "OSC message fields are missing.";
    return false;
  }
  message.address = object["address"].as<const char*>();
  message.value = object["value"].as<const char*>();
  const int type = object["type"].as<int>();
  if (type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING) {
    error = "OSC message type is invalid.";
    return false;
  }
  message.type = static_cast<OscValueType>(type);
  if (message.value.length() > 128) {
    error = "E_OSC_VALUE_TOO_LONG";
    return false;
  }
  if (!inputOscMessageValid(message)) {
    error = "OSC message address or value is invalid.";
    return false;
  }
  return true;
}

bool jsonSequence(JsonObjectConst object, SequenceSetting& sequence,
                  String& error) {
  if (object.isNull() || !object["address"].is<const char*>() ||
      !object["type"].is<int>() || object["start"].isNull() ||
      object["end"].isNull() || object["step"].isNull()) {
    error = "Sequence fields are missing.";
    return false;
  }
  sequence.address = object["address"].as<const char*>();
  const int type = object["type"].as<int>();
  sequence.start = object["start"].as<float>();
  sequence.end = object["end"].as<float>();
  sequence.step = object["step"].as<float>();
  if (type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING) {
    error = "Sequence values are invalid.";
    return false;
  }
  sequence.type = static_cast<OscValueType>(type);
  if (!isfinite(sequence.start) || !isfinite(sequence.end) ||
      !isfinite(sequence.step)) {
    error = "Sequence values are invalid.";
    return false;
  }
  if (sequence.step == 0.0f) {
    error = "E_SEQUENCE_STEP_ZERO";
    return false;
  }
  if ((sequence.start < sequence.end && sequence.step < 0.0f) ||
      (sequence.start > sequence.end && sequence.step > 0.0f)) {
    error = "E_SEQUENCE_DIRECTION_INVALID";
    return false;
  }
  if (!inputSequenceSettingValid(sequence)) {
    error = "Sequence values are invalid.";
    return false;
  }
  sequence.current = sequence.start;
  return true;
}

bool jsonButton(JsonObjectConst object, ButtonInputSetting& button,
                String& error, const char* modeKey = "mode") {
  if (object.isNull() || !object[modeKey].is<int>() ||
      !object["press"].is<JsonArrayConst>() ||
      !object["release"].is<JsonArrayConst>()) {
    error = "Button settings are missing.";
    return false;
  }
  const int mode = object[modeKey].as<int>();
  JsonArrayConst press = object["press"].as<JsonArrayConst>();
  JsonArrayConst release = object["release"].as<JsonArrayConst>();
  if (mode < INPUT_MODE_PRESS_RELEASE || mode > INPUT_MODE_SEQUENCE ||
      press.size() + release.size() > MAX_OSC_MESSAGES) {
    error = "Button mode or message count is invalid.";
    return false;
  }
  button.mode = static_cast<InputMode>(mode);
  button.pressMessageCount = static_cast<uint8_t>(press.size());
  button.releaseMessageCount = static_cast<uint8_t>(release.size());
  uint8_t index = 0;
  for (JsonObjectConst message : press)
    if (!jsonMessage(message, button.pressMessages[index++], error)) return false;
  index = 0;
  for (JsonObjectConst message : release)
    if (!jsonMessage(message, button.releaseMessages[index++], error)) return false;
  return jsonSequence(object["sequence"].as<JsonObjectConst>(), button.sequence,
                      error) && inputButtonSettingValid(button);
}
}  // namespace

bool inputValidateDevicePreset(JsonObjectConst root, int expectedDeviceType,
                               bool legacy, String& error) {
  if (!root.containsKey("deviceTypeName"))
    return fail(error, "E_PRESET_REQUIRED_FIELD_MISSING");
  if (!root["deviceTypeName"].is<const char*>())
    return fail(error, "E_PRESET_FIELD_TYPE_INVALID");

  if (expectedDeviceType == CHAIN_KEY_DEVICE_TYPE) {
    if (!root.containsKey("key"))
      return fail(error, "E_PRESET_REQUIRED_FIELD_MISSING");
    const JsonObjectConst key = root["key"].as<JsonObjectConst>();
    if (key.isNull()) return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    static const char* const required[] = {"mode", "press", "release",
                                           "sequence"};
    if (!requiredFields(key, required, 4, error)) return false;
    if (!key["mode"].is<int>() || !key["press"].is<JsonArrayConst>() ||
        !key["release"].is<JsonArrayConst>() ||
        !key["sequence"].is<JsonObjectConst>())
      return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    const int mode = key["mode"].as<int>();
    if (mode < INPUT_MODE_PRESS_RELEASE || mode > INPUT_MODE_SEQUENCE)
      return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
    return validateMessages(key["press"], key["release"], error) &&
           validateSequence(key["sequence"].as<JsonObjectConst>(), legacy,
                            error);
  }

  if (expectedDeviceType == CHAIN_ENCODER_DEVICE_TYPE) {
    if (!root.containsKey("encoder"))
      return fail(error, "E_PRESET_REQUIRED_FIELD_MISSING");
    const JsonObjectConst encoder = root["encoder"].as<JsonObjectConst>();
    if (encoder.isNull()) return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    if ((root["schemaVersion"] | INPUT_JSON_SCHEMA_VERSION) ==
        DEVICE_PRESET_V2_SCHEMA_VERSION)
      return validateEncoderV2(encoder, error);
    static const char* const required[] = {
        "rotationAddress", "sendIncrement", "absoluteInputMin",
        "absoluteInputMax", "incrementScale", "range", "clickMode",
        "press", "release", "sequence"};
    if (!requiredFields(encoder, required, 10, error)) return false;
    if (!encoder["rotationAddress"].is<const char*>() ||
        !encoder["sendIncrement"].is<bool>() ||
        (encoder.containsKey("wrapAround") && !encoder["wrapAround"].is<bool>()) ||
        !encoder["absoluteInputMin"].is<float>() ||
        !encoder["absoluteInputMax"].is<float>() ||
        !encoder["incrementScale"].is<float>() ||
        !encoder["range"].is<JsonObjectConst>() ||
        !encoder["clickMode"].is<int>() ||
        !encoder["press"].is<JsonArrayConst>() ||
        !encoder["release"].is<JsonArrayConst>() ||
        !encoder["sequence"].is<JsonObjectConst>())
      return fail(error, "E_PRESET_FIELD_TYPE_INVALID");
    if (!validPresetAddress(encoder["rotationAddress"].as<const char*>(),
                            error))
      return false;
    const float inputMin = encoder["absoluteInputMin"].as<float>();
    const float inputMax = encoder["absoluteInputMax"].as<float>();
    const float scale = encoder["incrementScale"].as<float>();
    const int mode = encoder["clickMode"].as<int>();
    if (!isfinite(inputMin) || !isfinite(inputMax) || inputMax <= inputMin ||
        !isfinite(scale) || mode < INPUT_MODE_PRESS_RELEASE ||
        mode > INPUT_MODE_SEQUENCE)
      return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
    return validateRange(encoder["range"].as<JsonObjectConst>(), legacy,
                         error) &&
           validateMessages(encoder["press"], encoder["release"], error) &&
           validateSequence(encoder["sequence"].as<JsonObjectConst>(), legacy,
                            error);
  }
  return fail(error, "E_PRESET_DEVICE_TYPE_UNSUPPORTED");
}

void inputNormalizeLegacyPresetTypes(JsonObject root, int deviceType) {
  if (deviceType == CHAIN_KEY_DEVICE_TYPE) {
    JsonObject sequence = root["key"]["sequence"].as<JsonObject>();
    const int type = sequence["type"] | OSC_TYPE_FLOAT;
    if (type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING)
      sequence["type"] = OSC_TYPE_FLOAT;
  } else if (deviceType == CHAIN_ENCODER_DEVICE_TYPE) {
    JsonObject sequence = root["encoder"]["sequence"].as<JsonObject>();
    const int type = sequence["type"] | OSC_TYPE_FLOAT;
    if (type < OSC_TYPE_FLOAT || type > OSC_TYPE_STRING)
      sequence["type"] = OSC_TYPE_FLOAT;
  }
}

String inputJsonQuote(const String& value) {
  String output = "\"";
  for (size_t index = 0; index < value.length(); ++index) {
    const uint8_t c = static_cast<uint8_t>(value[index]);
    if (c == '"') output += "\\\"";
    else if (c == '\\') output += "\\\\";
    else if (c == '\b') output += "\\b";
    else if (c == '\f') output += "\\f";
    else if (c == '\n') output += "\\n";
    else if (c == '\r') output += "\\r";
    else if (c == '\t') output += "\\t";
    else if (c < 0x20) {
      char escaped[7];
      snprintf(escaped, sizeof(escaped), "\\u%04X", c);
      output += escaped;
    } else output += static_cast<char>(c);
  }
  output += '"';
  return output;
}

String inputKeyJson(const KeyInputSetting& setting, uint8_t index,
                    bool includeIdentity) {
  String output = "{";
  if (includeIdentity) {
    output += String("\"identity\":\"chainoscpad:key:") + String(index + 1) +
              "\",\"deviceType\":3,\"deviceTypeName\":\"Key\"," +
              "\"displayName\":" + inputJsonQuote(setting.displayName) +
              ",\"builtIn\":true";
  } else {
    output += String("\"format\":") + inputJsonQuote(CHAINOSC_PRESET_FORMAT) +
              ",\"schemaVersion\":1,\"deviceType\":3," +
              "\"deviceTypeName\":\"Key\"";
  }
  output += ",\"key\":" + buttonJson(setting.button) + "}";
  return output;
}

String inputEncoderJson(const EncoderInputSetting& setting,
                        bool includeIdentity) {
  String output = "{";
  if (includeIdentity) {
    output += String("\"identity\":\"chainoscpad:encoder\"," 
              "\"deviceType\":1,\"deviceTypeName\":\"Encoder\"," 
              "\"displayName\":") + inputJsonQuote(setting.displayName) +
              ",\"builtIn\":true";
  } else {
    output += String("\"format\":") + inputJsonQuote(CHAINOSC_PRESET_FORMAT) +
              ",\"schemaVersion\":2,\"deviceType\":1," +
              "\"deviceTypeName\":\"Encoder\"";
  }
  output += String(",\"encoder\":{\"rotationAddress\":") +
            inputJsonQuote(setting.rotationAddress);
  if (setting.rotationMode == ENCODER_ROTATION_AMOUNT) {
    output += String(",\"rotationMode\":\"amount\",\"rangeSteps\":") +
              String(setting.rangeSteps) + ",\"wrap\":" +
              String(setting.wrapAround ? "true" : "false") +
              ",\"clockwiseIncreases\":" +
              String(setting.clockwiseIncreases ? "true" : "false") +
              ",\"outputMin\":" + String(setting.outputMin, 7) +
              ",\"outputMax\":" + String(setting.outputMax, 7);
  } else {
    String clockwise = setting.outputType == OSC_TYPE_STRING
                           ? inputJsonQuote(setting.clockwiseValue)
                           : setting.clockwiseValue;
    String counterClockwise = setting.outputType == OSC_TYPE_STRING
                                  ? inputJsonQuote(setting.counterClockwiseValue)
                                  : setting.counterClockwiseValue;
    output += String(",\"rotationMode\":\"direction\",\"clockwiseValue\":") +
              clockwise + ",\"counterClockwiseValue\":" + counterClockwise;
  }
  output += String(",\"outputType\":") +
            String(static_cast<int>(setting.outputType)) +
            ",\"pushMode\":" + String(static_cast<int>(setting.push.mode)) +
            ",\"press\":" +
            messageArrayJson(setting.push.pressMessages,
                             setting.push.pressMessageCount) +
            ",\"release\":" +
            messageArrayJson(setting.push.releaseMessages,
                             setting.push.releaseMessageCount) +
            ",\"sequence\":" + sequenceJson(setting.push.sequence) + "}}";
  return output;
}

bool inputKeyFromJson(JsonObjectConst object, KeyInputSetting& setting,
                      bool includeIdentity, uint8_t expectedIndex,
                      String& error) {
  if (object.isNull() || (object["deviceType"] | -1) != CHAIN_KEY_DEVICE_TYPE) {
    error = "Device type is not Key.";
    return false;
  }
  if (includeIdentity) {
    if (!object["identity"].is<const char*>() ||
        !object["displayName"].is<const char*>()) {
      error = "Device identity or name is missing.";
      return false;
    }
    const String expected = String("chainoscpad:key:") + String(expectedIndex + 1);
    if (String(object["identity"].as<const char*>()) != expected) {
      error = "Key identity does not match its position.";
      return false;
    }
    setting.displayName = object["displayName"].as<const char*>();
    setting.displayName.trim();
    if (setting.displayName.isEmpty() || setting.displayName.length() > 64) {
      error = "Device name is invalid.";
      return false;
    }
  }
  return jsonButton(object["key"].as<JsonObjectConst>(), setting.button, error);
}

bool inputEncoderFromJson(JsonObjectConst object, EncoderInputSetting& setting,
                          bool includeIdentity, String& error) {
  if (object.isNull() ||
      (object["deviceType"] | -1) != CHAIN_ENCODER_DEVICE_TYPE) {
    error = "Device type is not Encoder.";
    return false;
  }
  if (includeIdentity) {
    if (!object["identity"].is<const char*>() ||
        String(object["identity"].as<const char*>()) != "chainoscpad:encoder" ||
        !object["displayName"].is<const char*>()) {
      error = "Encoder identity or name is invalid.";
      return false;
    }
    setting.displayName = object["displayName"].as<const char*>();
    setting.displayName.trim();
    if (setting.displayName.isEmpty() || setting.displayName.length() > 64) {
      error = "Device name is invalid.";
      return false;
    }
  }
  JsonObjectConst encoder = object["encoder"].as<JsonObjectConst>();
  if (encoder.isNull() || !encoder["rotationAddress"].is<const char*>() ||
      !encoder["press"].is<JsonArrayConst>() ||
      !encoder["release"].is<JsonArrayConst>()) {
    error = "Encoder settings are missing.";
    return false;
  }
  setting.rotationAddress = encoder["rotationAddress"].as<const char*>();
  if (encoder.containsKey("rotationMode")) {
    const String mode = encoder["rotationMode"].as<const char*>();
    setting.outputType =
        static_cast<OscValueType>(encoder["outputType"].as<int>());
    if (mode == "amount") {
      setting.rotationMode = ENCODER_ROTATION_AMOUNT;
      setting.rangeSteps = static_cast<uint16_t>(encoder["rangeSteps"].as<int>());
      setting.wrapAround = encoder["wrap"].as<bool>();
      setting.clockwiseIncreases = encoder["clockwiseIncreases"].as<bool>();
      setting.outputMin = encoder["outputMin"].as<float>();
      setting.outputMax = encoder["outputMax"].as<float>();
    } else {
      setting.rotationMode = ENCODER_ROTATION_DIRECTION;
      if (setting.outputType == OSC_TYPE_STRING) {
        setting.clockwiseValue = encoder["clockwiseValue"].as<const char*>();
        setting.counterClockwiseValue =
            encoder["counterClockwiseValue"].as<const char*>();
      } else if (setting.outputType == OSC_TYPE_INT) {
        setting.clockwiseValue = String(encoder["clockwiseValue"].as<int32_t>());
        setting.counterClockwiseValue =
            String(encoder["counterClockwiseValue"].as<int32_t>());
      } else {
        setting.clockwiseValue =
            String(encoder["clockwiseValue"].as<float>(), 7);
        setting.counterClockwiseValue =
            String(encoder["counterClockwiseValue"].as<float>(), 7);
      }
    }
  } else {
    JsonObjectConst range = encoder["range"].as<JsonObjectConst>();
    if (range.isNull()) { error = "Encoder range is missing."; return false; }
    const bool increment = encoder["sendIncrement"] | false;
    const float inputMin = encoder["absoluteInputMin"].as<float>();
    const float inputMax = encoder["absoluteInputMax"].as<float>();
    const float scale = encoder["incrementScale"].as<float>();
    setting.outputMin = range["outMin"].as<float>();
    setting.outputMax = range["outMax"].as<float>();
    setting.outputType = static_cast<OscValueType>(range["type"].as<int>());
    setting.wrapAround = encoder["wrapAround"] | true;
    setting.clockwiseIncreases = true;
    if (increment) {
      setting.rotationMode = ENCODER_ROTATION_DIRECTION;
      const float low = min(setting.outputMin, setting.outputMax);
      const float high = max(setting.outputMin, setting.outputMax);
      const float clockwise = constrain(scale, low, high);
      const float counterClockwise = constrain(-scale, low, high);
      if (setting.outputType == OSC_TYPE_INT) {
        setting.clockwiseValue = String(static_cast<int32_t>(lroundf(clockwise)));
        setting.counterClockwiseValue = String(static_cast<int32_t>(lroundf(counterClockwise)));
      } else if (setting.outputType == OSC_TYPE_STRING) {
        setting.clockwiseValue = String(clockwise, 3);
        setting.counterClockwiseValue = String(counterClockwise, 3);
        if (setting.clockwiseValue == "-0.000") setting.clockwiseValue = "0.000";
        if (setting.counterClockwiseValue == "-0.000") setting.counterClockwiseValue = "0.000";
      } else {
        setting.clockwiseValue = String(clockwise, 7);
        setting.counterClockwiseValue = String(counterClockwise, 7);
      }
    } else {
      const float span = inputMax - inputMin;
      if (!isfinite(span) || span < 1.0f || span > 65535.0f ||
          floorf(span) != span) {
        error = "E_PRESET_DEVICE_SETTING_INVALID";
        return false;
      }
      setting.rotationMode = ENCODER_ROTATION_AMOUNT;
      setting.rangeSteps = static_cast<uint16_t>(span);
    }
  }
  const bool v2 = encoder.containsKey("rotationMode");
  if (!jsonButton(encoder, setting.push, error,
                  v2 ? "pushMode" : "clickMode"))
    return false;
  if (!inputEncoderSettingValid(setting))
    return fail(error, "E_PRESET_DEVICE_SETTING_INVALID");
  return true;
}
