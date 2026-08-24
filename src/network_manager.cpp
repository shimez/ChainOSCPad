#include "network_manager.h"

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

#include "config.h"

namespace {

DNSServer dnsServer;
WebServer webServer(80);

String configuredSsid;
String configuredPassword;
String oscHost = OSC_TARGET_HOST;
uint16_t oscPort = OSC_TARGET_PORT;
bool accessPointMode = false;
bool mdnsStarted = false;
bool restartPending = false;
uint32_t restartAtMs = 0;

String htmlEscape(const String& value) {
  String escaped;
  escaped.reserve(value.length() + 16);
  for (size_t i = 0; i < value.length(); ++i) {
    switch (value[i]) {
      case '&': escaped += F("&amp;"); break;
      case '<': escaped += F("&lt;"); break;
      case '>': escaped += F("&gt;"); break;
      case '"': escaped += F("&quot;"); break;
      case '\'': escaped += F("&#39;"); break;
      default: escaped += value[i]; break;
    }
  }
  return escaped;
}

bool validOscHost(const String& host) {
  if (host.isEmpty() || host.length() > 253) return false;
  for (size_t i = 0; i < host.length(); ++i) {
    if (isWhitespace(host[i]) || host[i] == '/' || host[i] == ':') return false;
  }
  return true;
}

void loadSettings() {
  Preferences preferences;
  if (preferences.begin(PREFS_NAMESPACE, true)) {
    configuredSsid = preferences.getString("ssid", "");
    configuredPassword = preferences.getString("password", "");
    oscHost = preferences.getString("osc_host", OSC_TARGET_HOST);
    const uint32_t storedPort = preferences.getUInt("osc_port", OSC_TARGET_PORT);
    if (storedPort >= 1 && storedPort <= 65535) {
      oscPort = static_cast<uint16_t>(storedPort);
    }
    preferences.end();
  }

}

bool saveSettings(const String& ssid, const String& password,
                  const String& host, uint16_t port) {
  Preferences preferences;
  if (!preferences.begin(PREFS_NAMESPACE, false)) return false;
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putString("osc_host", host);
  preferences.putUInt("osc_port", port);
  preferences.end();

  if (!preferences.begin(PREFS_NAMESPACE, true)) return false;
  const bool ok = preferences.getString("ssid", "") == ssid &&
                  preferences.getString("password", "") == password &&
                  preferences.getString("osc_host", "") == host &&
                  preferences.getUInt("osc_port", 0) == port;
  preferences.end();
  return ok;
}

String page(const String& message = "", bool error = false) {
  const bool connected = WiFi.status() == WL_CONNECTED;
  String body;
  body.reserve(5000);
  body += F("<!doctype html><html lang='ja'><head><meta charset='utf-8'>");
  body += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  body += F("<title>ChainOSCPad Settings</title><style>");
  body += F("body{font-family:system-ui,sans-serif;background:#f4f6f8;color:#1f2937;margin:0;padding:20px}");
  body += F("main{max-width:620px;margin:auto;background:#fff;padding:24px;border-radius:14px;box-shadow:0 8px 30px #0001}");
  body += F("h1{margin-top:0}label{display:block;margin-top:16px;font-weight:600}");
  body += F("input{box-sizing:border-box;width:100%;padding:11px;margin-top:6px;border:1px solid #ccd3dc;border-radius:8px;font-size:16px}");
  body += F("button{margin-top:22px;padding:11px 18px;border:0;border-radius:8px;background:#2563eb;color:#fff;font-size:15px;cursor:pointer}");
  body += F(".danger{background:#b91c1c}.status{padding:12px;border-radius:8px;background:#eef2ff}.ok{color:#166534}.err{color:#b91c1c}.hint{color:#64748b;font-size:14px}</style></head><body><main>");
  body += F("<h1>ChainOSCPad</h1><p class='status'>Version ");
  body += APP_VERSION;
  body += connected ? F(" / Wi-Fi接続済み / IP: ") : F(" / 設定APモード / IP: ");
  body += connected ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  body += F("</p>");
  if (!message.isEmpty()) {
    body += error ? F("<p class='err'>") : F("<p class='ok'>");
    body += htmlEscape(message);
    body += F("</p>");
  }
  body += F("<form method='post' action='/save'><h2>Wi-Fi</h2><label>SSID<input name='ssid' maxlength='32' required value='");
  body += htmlEscape(configuredSsid);
  body += F("'></label><label>Password<input name='password' type='password' maxlength='64' placeholder='変更しない場合は空欄'></label>");
  body += F("<p class='hint'>保存済みパスワードは画面に表示しません。</p><h2>OSC送信先</h2>");
  body += F("<label>Host / IP address<input name='host' maxlength='253' required value='");
  body += htmlEscape(oscHost);
  body += F("'></label><label>Port<input name='port' type='number' min='1' max='65535' required value='");
  body += String(oscPort);
  body += F("'></label><button type='submit'>保存して再起動</button></form>");
  body += F("<form method='post' action='/reset' onsubmit=\"return confirm('保存設定を削除しますか？')\"><button class='danger' type='submit'>設定をリセット</button></form>");
  body += F("<p class='hint'>信頼できるローカルネットワークで使用してください。この設定画面に認証機能はありません。</p>");
  body += F("</main></body></html>");
  return body;
}

void scheduleRestart() {
  restartPending = true;
  restartAtMs = millis() + NETWORK_RESTART_DELAY_MS;
}

void setupWebServer() {
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html; charset=utf-8", page());
  });

  webServer.on("/save", HTTP_POST, []() {
    String ssid = webServer.arg("ssid");
    String password = webServer.arg("password");
    String host = webServer.arg("host");
    ssid.trim();
    host.trim();
    const long portValue = webServer.arg("port").toInt();

    if (ssid.isEmpty() || ssid.length() > 32 || password.length() > 64 ||
        !validOscHost(host) || portValue < 1 || portValue > 65535) {
      webServer.send(400, "text/html; charset=utf-8",
                     page("入力内容を確認してください。", true));
      return;
    }
    // Empty means "keep the password" only when editing the same SSID. For a
    // new SSID it intentionally represents an open network.
    if (password.isEmpty() && ssid == configuredSsid) {
      password = configuredPassword;
    }
    if (!saveSettings(ssid, password, host, static_cast<uint16_t>(portValue))) {
      webServer.send(500, "text/html; charset=utf-8",
                     page("設定を保存できませんでした。", true));
      return;
    }
    webServer.send(200, "text/html; charset=utf-8",
                   page("設定を保存しました。再起動します。"));
    scheduleRestart();
  });

  webServer.on("/reset", HTTP_POST, []() {
    Preferences preferences;
    bool cleared = false;
    if (preferences.begin(PREFS_NAMESPACE, false)) {
      cleared = preferences.clear();
      preferences.end();
    }
    if (!cleared) {
      webServer.send(500, "text/html; charset=utf-8",
                     page("設定を削除できませんでした。", true));
      return;
    }
    webServer.send(200, "text/html; charset=utf-8",
                   page("設定を削除しました。再起動します。"));
    scheduleRestart();
  });

  // Common captive-portal probes.
  webServer.on("/generate_204", HTTP_ANY, []() { webServer.sendHeader("Location", "/", true); webServer.send(302, "text/plain", ""); });
  webServer.on("/hotspot-detect.html", HTTP_ANY, []() { webServer.sendHeader("Location", "/", true); webServer.send(302, "text/plain", ""); });
  webServer.on("/connecttest.txt", HTTP_ANY, []() { webServer.sendHeader("Location", "/", true); webServer.send(302, "text/plain", ""); });
  webServer.onNotFound([]() {
    if (accessPointMode) {
      webServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
      webServer.send(302, "text/plain", "");
    } else {
      webServer.send(404, "text/plain", "Not found");
    }
  });
  webServer.begin();
  Serial.println("[Web] HTTP server started");
}

void startAccessPoint() {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_AP);
  accessPointMode = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  if (!accessPointMode) {
    Serial.println("[WiFi] Failed to start setup AP");
    return;
  }
  dnsServer.start(53, "*", WiFi.softAPIP());
  Serial.printf("[WiFi] Setup AP=%s IP=%s password=%s\n", WIFI_AP_SSID,
                WiFi.softAPIP().toString().c_str(), WIFI_AP_PASSWORD);
}

bool connectStation() {
  if (configuredSsid.isEmpty()) return false;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(configuredSsid.c_str(), configuredPassword.c_str());
  Serial.printf("[WiFi] Connecting to %s", configuredSsid.c_str());
  const uint32_t startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) return false;
  Serial.printf("[WiFi] Connected IP=%s\n", WiFi.localIP().toString().c_str());
  mdnsStarted = MDNS.begin(WIFI_MDNS_HOST);
  if (mdnsStarted) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] http://%s.local/\n", WIFI_MDNS_HOST);
  }
  return true;
}

}  // namespace

void networkSetup() {
  loadSettings();
  if (!connectStation()) startAccessPoint();
  setupWebServer();
  Serial.printf("[OSC] target=%s:%u\n", oscHost.c_str(), oscPort);
}

void networkLoop() {
  if (accessPointMode) dnsServer.processNextRequest();
  webServer.handleClient();
  if (restartPending && static_cast<int32_t>(millis() - restartAtMs) >= 0) {
    ESP.restart();
  }
}

bool networkIsConnected() { return WiFi.status() == WL_CONNECTED; }
bool networkIsAccessPoint() { return accessPointMode; }
const String& networkOscHost() { return oscHost; }
uint16_t networkOscPort() { return oscPort; }
