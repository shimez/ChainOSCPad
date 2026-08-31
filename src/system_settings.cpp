#include "system_settings.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "config.h"

namespace {
constexpr char SETTINGS_PATH[] = "/system/settings.json";
constexpr char TEMP_PATH[] = "/system/settings.json.tmp";
constexpr char BACKUP_PATH[] = "/system/settings.json.bak";
constexpr char FORMAT[] = "ChainOSCPad-system-settings";
constexpr uint8_t FILE_VERSION = 1;

struct Settings {
  String ssid;
  String password;
  String oscHost = OSC_TARGET_HOST;
  uint16_t oscPort = OSC_TARGET_PORT;
  bool languageConfigured = false;
  uint8_t language = 0;
};

Settings settings;

void logResult(const char* operation, const char* result, const char* reason,
               size_t fileBytes = 0) {
  Serial.printf(
      "[ChainOSCPad][SYSTEM] operation=%s path=%s file_bytes=%u "
      "total_bytes=%u used_bytes=%u free_bytes=%u result=%s reason=%s\n",
      operation, SETTINGS_PATH, static_cast<unsigned>(fileBytes),
      static_cast<unsigned>(LittleFS.totalBytes()),
      static_cast<unsigned>(LittleFS.usedBytes()),
      static_cast<unsigned>(LittleFS.totalBytes() - LittleFS.usedBytes()),
      result, reason);
}

bool decode(JsonObjectConst root, Settings& result) {
  if (String(root["format"] | "") != FORMAT ||
      root["version"].as<int>() != FILE_VERSION)
    return false;
  JsonObjectConst wifi = root["wifi"].as<JsonObjectConst>();
  JsonObjectConst osc = root["osc"].as<JsonObjectConst>();
  JsonObjectConst ui = root["ui"].as<JsonObjectConst>();
  if (wifi.isNull() || osc.isNull() || ui.isNull() ||
      !wifi["ssid"].is<const char*>() ||
      !wifi["password"].is<const char*>() ||
      !osc["host"].is<const char*>() || !osc["port"].is<int>() ||
      !ui["configured"].is<bool>() || !ui["language"].is<int>())
    return false;
  const int port = osc["port"].as<int>();
  const int language = ui["language"].as<int>();
  if (port < 1 || port > 65535 || language < 0 || language > 1) return false;
  result.ssid = wifi["ssid"].as<const char*>();
  result.password = wifi["password"].as<const char*>();
  result.oscHost = osc["host"].as<const char*>();
  result.oscPort = static_cast<uint16_t>(port);
  result.languageConfigured = ui["configured"].as<bool>();
  result.language = static_cast<uint8_t>(language);
  return !result.oscHost.isEmpty();
}

bool loadFile(const char* path, Settings& result) {
  if (!LittleFS.exists(path)) return false;
  File file = LittleFS.open(path, FILE_READ);
  if (!file) return false;
  const size_t bytes = file.size();
  JsonDocument document;
  const DeserializationError error = deserializeJson(document, file);
  file.close();
  const bool ok = !error && decode(document.as<JsonObjectConst>(), result);
  logResult("load", ok ? "ok" : "failed",
            error ? "parse_failed" : (ok ? "none" : "validation_failed"),
            bytes);
  return ok;
}

bool writeFile(const Settings& value) {
  JsonDocument document;
  JsonObject root = document.to<JsonObject>();
  root["format"] = FORMAT;
  root["version"] = FILE_VERSION;
  JsonObject wifi = root["wifi"].to<JsonObject>();
  wifi["ssid"] = value.ssid;
  wifi["password"] = value.password;
  JsonObject osc = root["osc"].to<JsonObject>();
  osc["host"] = value.oscHost;
  osc["port"] = value.oscPort;
  JsonObject ui = root["ui"].to<JsonObject>();
  ui["configured"] = value.languageConfigured;
  ui["language"] = value.language;

  LittleFS.mkdir("/system");
  LittleFS.remove(TEMP_PATH);
  File file = LittleFS.open(TEMP_PATH, FILE_WRITE);
  if (!file) {
    logResult("save", "failed", "temp_open_failed");
    return false;
  }
  const size_t written = serializeJson(document, file);
  file.flush();
  const size_t fileBytes = file.size();
  file.close();
  Settings verified;
  if (written == 0 || fileBytes != written || !loadFile(TEMP_PATH, verified)) {
    LittleFS.remove(TEMP_PATH);
    logResult("save", "failed", "temp_verify_failed", fileBytes);
    return false;
  }

  LittleFS.remove(BACKUP_PATH);
  const bool hadCurrent = LittleFS.exists(SETTINGS_PATH);
  if (hadCurrent && !LittleFS.rename(SETTINGS_PATH, BACKUP_PATH)) {
    LittleFS.remove(TEMP_PATH);
    logResult("save", "failed", "backup_failed", fileBytes);
    return false;
  }
  if (!LittleFS.rename(TEMP_PATH, SETTINGS_PATH)) {
    if (hadCurrent) LittleFS.rename(BACKUP_PATH, SETTINGS_PATH);
    LittleFS.remove(TEMP_PATH);
    logResult("save", "failed", "replace_failed", fileBytes);
    return false;
  }
  LittleFS.remove(BACKUP_PATH);
  logResult("save", "ok", "none", fileBytes);
  return true;
}

Settings loadLegacyNvs() {
  Settings legacy;
  Preferences preferences;
  if (!preferences.begin(PREFS_NAMESPACE, true)) return legacy;
  legacy.ssid = preferences.getString("ssid", "");
  legacy.password = preferences.getString("password", "");
  legacy.oscHost = preferences.getString("osc_host", OSC_TARGET_HOST);
  const uint32_t port = preferences.getUInt("osc_port", OSC_TARGET_PORT);
  if (port >= 1 && port <= 65535) legacy.oscPort = port;
  const uint8_t language = preferences.getUChar("language", 0xff);
  if (language <= 1) {
    legacy.languageConfigured = true;
    legacy.language = language;
  }
  preferences.end();
  return legacy;
}

bool update(const Settings& candidate) {
  if (!writeFile(candidate)) return false;
  settings = candidate;
  return true;
}
}  // namespace

bool systemSettingsSetup() {
  if (!LittleFS.begin(false)) {
    logResult("mount", "failed", "mount_failed");
    return false;
  }
  Settings loaded;
  if (loadFile(SETTINGS_PATH, loaded)) {
    settings = loaded;
    return true;
  }
  if (loadFile(BACKUP_PATH, loaded)) {
    settings = loaded;
    writeFile(settings);
    return true;
  }
  settings = loadLegacyNvs();
  const bool saved = writeFile(settings);
  logResult("migrate", saved ? "ok" : "failed",
            saved ? "legacy_nvs" : "write_failed");
  return saved;
}

const String& systemSettingsWifiSsid() { return settings.ssid; }
const String& systemSettingsWifiPassword() { return settings.password; }
const String& systemSettingsOscHost() { return settings.oscHost; }
uint16_t systemSettingsOscPort() { return settings.oscPort; }
bool systemSettingsLanguageConfigured() { return settings.languageConfigured; }
uint8_t systemSettingsLanguage() { return settings.language; }

bool systemSettingsSaveWifi(const String& ssid, const String& password) {
  Settings candidate = settings;
  candidate.ssid = ssid;
  candidate.password = password;
  return update(candidate);
}

bool systemSettingsClearWifi() {
  return systemSettingsSaveWifi(String(), String());
}

bool systemSettingsSaveOsc(const String& host, uint16_t port) {
  Settings candidate = settings;
  candidate.oscHost = host;
  candidate.oscPort = port;
  return update(candidate);
}

bool systemSettingsSaveLanguage(uint8_t language) {
  if (language > 1) return false;
  Settings candidate = settings;
  candidate.languageConfigured = true;
  candidate.language = language;
  return update(candidate);
}

bool systemSettingsClearAll() {
  bool ok = true;
  if (LittleFS.exists(SETTINGS_PATH)) ok = LittleFS.remove(SETTINGS_PATH) && ok;
  if (LittleFS.exists(TEMP_PATH)) ok = LittleFS.remove(TEMP_PATH) && ok;
  if (LittleFS.exists(BACKUP_PATH)) ok = LittleFS.remove(BACKUP_PATH) && ok;
  Preferences preferences;
  if (preferences.begin(PREFS_NAMESPACE, false)) {
    ok = preferences.clear() && ok;
    preferences.end();
  } else {
    ok = false;
  }
  settings = Settings();
  logResult("reset", ok ? "ok" : "failed", ok ? "none" : "remove_failed");
  return ok;
}
