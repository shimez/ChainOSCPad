#include <Arduino.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

namespace {

constexpr char AP_SSID[] = "ChainOSCPad-FS";
constexpr char AP_PASSWORD[] = "chainosc-fs";
constexpr uint16_t DNS_PORT = 53;
constexpr size_t EDIT_LIMIT = 16 * 1024;
constexpr size_t PATH_LIMIT = 192;

WebServer server(80);
DNSServer dnsServer;
File uploadFile;
String uploadPath;
String uploadTemporaryPath;
String uploadError;
bool uploadComplete = false;

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="ja"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ChainOSCPad LittleFS Browser</title><style>
:root{font-family:system-ui,sans-serif;color:#172033;background:#f3f5f8}body{max-width:1100px;margin:auto;padding:20px}h1{font-size:1.5rem}.card{background:#fff;border:1px solid #d8dee8;border-radius:12px;padding:16px;margin:12px 0}.bar{display:flex;gap:8px;flex-wrap:wrap;align-items:center}button,.button{border:1px solid #aeb9ca;border-radius:7px;background:#fff;padding:8px 12px;cursor:pointer;color:#172033;text-decoration:none}button.primary{background:#2563eb;color:#fff;border-color:#2563eb}button.danger{color:#c62828;border-color:#efb3b3}input,textarea,select{box-sizing:border-box;border:1px solid #aeb9ca;border-radius:7px;padding:8px;font:inherit}input[type=text]{min-width:280px}textarea{width:100%;height:340px;font-family:ui-monospace,monospace;white-space:pre}table{width:100%;border-collapse:collapse}th,td{text-align:left;padding:8px;border-bottom:1px solid #e3e7ee}small,.muted{color:#667085}.error{color:#b42318}.ok{color:#067647}@media(max-width:700px){th:nth-child(3),td:nth-child(3){display:none}.actions{display:flex;flex-wrap:wrap;gap:4px}}
</style></head><body><h1>ChainOSCPad LittleFS Browser</h1>
<div class="card"><div id="status">読み込み中...</div><small>このファームはLittleFSを自動フォーマットしません。</small></div>
<div class="card"><div class="bar"><button class="primary" onclick="refreshFiles()">再読み込み</button><input id="uploadPath" type="text" placeholder="/path/to/file.bin"><input id="uploadInput" type="file"><button onclick="uploadSelected()">アップロード／置換</button></div><div id="message"></div></div>
<div class="card"><table><thead><tr><th>パス</th><th>サイズ</th><th>種別</th><th>操作</th></tr></thead><tbody id="files"></tbody></table></div>
<div class="card" id="editorCard" hidden><h2 id="editorTitle">編集</h2><div class="bar"><select id="editorMode"><option value="text">テキスト</option><option value="hex">16進数</option></select><button onclick="reloadEditor()">再読込</button><button class="primary" onclick="saveEditor()">保存</button><button onclick="closeEditor()">閉じる</button></div><p class="muted">16進数は「00 01 FF」の形式です。最大16 KiBまでWeb編集できます。それ以上はダウンロード／アップロードを使用してください。</p><textarea id="editor"></textarea></div>
<script>
let currentPath='';const $=id=>document.getElementById(id);const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
function msg(text,ok=false){$('message').className=ok?'ok':'error';$('message').textContent=text}
async function request(url,opt){const r=await fetch(url,opt);const text=await r.text();if(!r.ok)throw new Error(text||('HTTP '+r.status));return text}
async function refreshFiles(){try{const info=JSON.parse(await request('/api/info'));$('status').textContent=`LittleFS: ${info.used} / ${info.total} bytes 使用、空き ${info.free} bytes`;const list=JSON.parse(await request('/api/files'));$('files').innerHTML=list.map(f=>`<tr><td>${esc(f.path)}</td><td>${f.size}</td><td>${f.directory?'directory':'file'}</td><td class="actions">${f.directory?'':`<button onclick="editFile('${encodeURIComponent(f.path)}','text')">Text</button><button onclick="editFile('${encodeURIComponent(f.path)}','hex')">Hex</button><a class="button" href="/api/download?path=${encodeURIComponent(f.path)}">Download</a><button class="danger" onclick="deleteFile('${encodeURIComponent(f.path)}')">削除</button>`}</td></tr>`).join('')||'<tr><td colspan="4">ファイルはありません</td></tr>'}catch(e){msg(e.message)}}
async function editFile(encoded,mode){currentPath=decodeURIComponent(encoded);$('editorMode').value=mode;$('editorTitle').textContent='編集: '+currentPath;$('editorCard').hidden=false;await reloadEditor();$('editorCard').scrollIntoView({behavior:'smooth'})}
async function reloadEditor(){if(!currentPath)return;try{const mode=$('editorMode').value;$('editor').value=await request(`/api/edit?path=${encodeURIComponent(currentPath)}&mode=${mode}`)}catch(e){msg(e.message)}}
async function saveEditor(){if(!currentPath)return;try{const mode=$('editorMode').value;await request(`/api/save?path=${encodeURIComponent(currentPath)}&mode=${mode}`,{method:'POST',headers:{'Content-Type':'text/plain'},body:$('editor').value});msg('保存しました: '+currentPath,true);await refreshFiles()}catch(e){msg(e.message)}}
function closeEditor(){$('editorCard').hidden=true;currentPath=''}
async function deleteFile(encoded){const path=decodeURIComponent(encoded);if(!confirm(path+' を削除しますか？'))return;try{await request('/api/file?path='+encodeURIComponent(path),{method:'DELETE'});msg('削除しました: '+path,true);await refreshFiles()}catch(e){msg(e.message)}}
async function uploadSelected(){const input=$('uploadInput'),path=$('uploadPath').value.trim();if(!path||!input.files.length){msg('保存先パスとファイルを選択してください');return}const form=new FormData();form.append('file',input.files[0]);try{await request('/api/upload?path='+encodeURIComponent(path),{method:'POST',body:form});msg('アップロードしました: '+path,true);await refreshFiles()}catch(e){msg(e.message)}}
refreshFiles();
</script></body></html>
)HTML";

String jsonEscape(const String& value) {
  String result;
  result.reserve(value.length() + 8);
  for (const char c : value) {
    if (c == '\\' || c == '"') { result += '\\'; result += c; }
    else if (c == '\n') result += "\\n";
    else if (c == '\r') result += "\\r";
    else if (static_cast<uint8_t>(c) < 0x20) result += '?';
    else result += c;
  }
  return result;
}

bool validPath(const String& path) {
  return path.length() > 1 && path.length() <= PATH_LIMIT && path[0] == '/' &&
         path.indexOf("..") < 0 && path.indexOf('\\') < 0 && path.indexOf("//") < 0;
}

void sendError(int status, const String& message) {
  Serial.printf("[Web] error=%d message=%s\n", status, message.c_str());
  server.send(status, "text/plain; charset=utf-8", message);
}

void appendDirectoryJson(File& directory, String& json, bool& first) {
  File entry = directory.openNextFile();
  while (entry) {
    const String path = entry.path();
    if (!first) json += ',';
    first = false;
    json += "{\"path\":\"" + jsonEscape(path) + "\",\"size\":" + String(entry.size()) +
            ",\"directory\":" + String(entry.isDirectory() ? "true" : "false") + "}";
    if (entry.isDirectory()) appendDirectoryJson(entry, json, first);
    entry.close();
    entry = directory.openNextFile();
  }
}

bool atomicReplace(const String& path, const uint8_t* data, size_t length) {
  const String temporary = path + ".maintenance.tmp";
  LittleFS.remove(temporary);
  File output = LittleFS.open(temporary, FILE_WRITE);
  if (!output) return false;
  const size_t written = output.write(data, length);
  output.flush();
  output.close();
  if (written != length) { LittleFS.remove(temporary); return false; }
  if (LittleFS.exists(path) && !LittleFS.remove(path)) { LittleFS.remove(temporary); return false; }
  if (!LittleFS.rename(temporary, path)) { LittleFS.remove(temporary); return false; }
  return true;
}

int hexNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

void setupRoutes() {
  server.on("/", HTTP_GET, [] { server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); });

  server.on("/api/info", HTTP_GET, [] {
    const size_t total = LittleFS.totalBytes(), used = LittleFS.usedBytes();
    server.send(200, "application/json", "{\"total\":" + String(total) + ",\"used\":" +
                String(used) + ",\"free\":" + String(total - used) + "}");
  });

  server.on("/api/files", HTTP_GET, [] {
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) return sendError(500, "LittleFS rootを開けません。");
    String json = "[";
    bool first = true;
    appendDirectoryJson(root, json, first);
    root.close();
    json += ']';
    server.send(200, "application/json", json);
  });

  server.on("/api/download", HTTP_GET, [] {
    const String path = server.arg("path");
    if (!validPath(path) || !LittleFS.exists(path)) return sendError(404, "ファイルが見つかりません。");
    File file = LittleFS.open(path, FILE_READ);
    if (!file || file.isDirectory()) return sendError(400, "ファイルではありません。");
    server.sendHeader("Content-Disposition", "attachment; filename=\"" + path.substring(path.lastIndexOf('/') + 1) + "\"");
    server.streamFile(file, "application/octet-stream");
    file.close();
  });

  server.on("/api/edit", HTTP_GET, [] {
    const String path = server.arg("path"), mode = server.arg("mode");
    if (!validPath(path) || !LittleFS.exists(path)) return sendError(404, "ファイルが見つかりません。");
    File file = LittleFS.open(path, FILE_READ);
    if (!file || file.isDirectory()) return sendError(400, "ファイルではありません。");
    if (file.size() > EDIT_LIMIT) { file.close(); return sendError(413, "Web編集の上限16 KiBを超えています。"); }
    String output;
    if (mode == "hex") {
      const char digits[] = "0123456789ABCDEF";
      output.reserve(file.size() * 3);
      size_t index = 0;
      while (file.available()) {
        const uint8_t value = file.read();
        output += digits[value >> 4]; output += digits[value & 0x0F];
        output += (++index % 16 == 0) ? '\n' : ' ';
      }
    } else {
      output.reserve(file.size());
      while (file.available()) output += static_cast<char>(file.read());
    }
    file.close();
    server.send(200, "text/plain; charset=utf-8", output);
  });

  server.on("/api/save", HTTP_POST, [] {
    const String path = server.arg("path"), mode = server.arg("mode"), body = server.arg("plain");
    if (!validPath(path)) return sendError(400, "無効なパスです。");
    bool ok = false;
    if (mode == "hex") {
      std::unique_ptr<uint8_t[]> bytes(new uint8_t[EDIT_LIMIT]);
      size_t count = 0;
      int high = -1;
      for (const char c : body) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        const int nibble = hexNibble(c);
        if (nibble < 0 || count >= EDIT_LIMIT) return sendError(400, "16進数の形式またはサイズが不正です。");
        if (high < 0) high = nibble;
        else { bytes[count++] = static_cast<uint8_t>((high << 4) | nibble); high = -1; }
      }
      if (high >= 0) return sendError(400, "16進数の桁数が奇数です。");
      ok = atomicReplace(path, bytes.get(), count);
    } else {
      if (body.length() > EDIT_LIMIT) return sendError(413, "Web編集の上限16 KiBを超えています。");
      ok = atomicReplace(path, reinterpret_cast<const uint8_t*>(body.c_str()), body.length());
    }
    if (!ok) return sendError(500, "保存に失敗しました。");
    Serial.printf("[LittleFS] saved path=%s bytes=%u\n", path.c_str(), static_cast<unsigned>(LittleFS.open(path).size()));
    server.send(200, "text/plain; charset=utf-8", "保存しました。");
  });

  server.on("/api/file", HTTP_DELETE, [] {
    const String path = server.arg("path");
    if (!validPath(path) || !LittleFS.exists(path)) return sendError(404, "ファイルが見つかりません。");
    File file = LittleFS.open(path, FILE_READ);
    const bool directory = file && file.isDirectory();
    file.close();
    if (directory) return sendError(400, "ディレクトリ削除には対応していません。");
    if (!LittleFS.remove(path)) return sendError(500, "削除に失敗しました。");
    Serial.printf("[LittleFS] removed path=%s\n", path.c_str());
    server.send(200, "text/plain; charset=utf-8", "削除しました。");
  });

  server.on("/api/upload", HTTP_POST, [] {
    if (!uploadComplete || uploadError.length()) return sendError(500, uploadError.length() ? uploadError : "アップロードに失敗しました。");
    if (LittleFS.exists(uploadPath) && !LittleFS.remove(uploadPath)) return sendError(500, "既存ファイルを置換できません。");
    if (!LittleFS.rename(uploadTemporaryPath, uploadPath)) return sendError(500, "一時ファイルを確定できません。");
    Serial.printf("[LittleFS] uploaded path=%s\n", uploadPath.c_str());
    server.send(200, "text/plain; charset=utf-8", "アップロードしました。");
  }, [] {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      uploadComplete = false; uploadError = ""; uploadPath = server.arg("path");
      if (!validPath(uploadPath)) { uploadError = "無効なパスです。"; return; }
      uploadTemporaryPath = uploadPath + ".maintenance.tmp";
      LittleFS.remove(uploadTemporaryPath);
      uploadFile = LittleFS.open(uploadTemporaryPath, FILE_WRITE);
      if (!uploadFile) uploadError = "一時ファイルを開けません。";
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (uploadError.length() || !uploadFile) return;
      if (uploadFile.write(upload.buf, upload.currentSize) != upload.currentSize) uploadError = "書き込み容量が不足しています。";
    } else if (upload.status == UPLOAD_FILE_END) {
      if (uploadFile) { uploadFile.flush(); uploadFile.close(); }
      if (uploadError.length()) LittleFS.remove(uploadTemporaryPath);
      else uploadComplete = true;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
      if (uploadFile) uploadFile.close();
      LittleFS.remove(uploadTemporaryPath); uploadError = "アップロードが中断されました。";
    }
  });

  server.onNotFound([] { server.sendHeader("Location", "/", true); server.send(302, "text/plain", ""); });
}

void printFiles(File& directory, uint8_t depth = 0) {
  File entry = directory.openNextFile();
  while (entry) {
    Serial.printf("[LittleFS] %*s%s%s bytes=%u\n", depth * 2, "", entry.path(), entry.isDirectory() ? "/" : "", static_cast<unsigned>(entry.size()));
    if (entry.isDirectory()) printFiles(entry, depth + 1);
    entry.close();
    entry = directory.openNextFile();
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nChainOSCPad LittleFS Browser (XIAO ESP32S3)");
  if (!LittleFS.begin(false)) {
    Serial.println("[LittleFS] mount failed; NOT formatted. Stop here to preserve evidence.");
    while (true) delay(1000);
  }
  Serial.printf("[LittleFS] mounted used=%u total=%u free=%u\n", static_cast<unsigned>(LittleFS.usedBytes()),
                static_cast<unsigned>(LittleFS.totalBytes()), static_cast<unsigned>(LittleFS.totalBytes() - LittleFS.usedBytes()));
  File root = LittleFS.open("/");
  if (root) { printFiles(root); root.close(); }

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD, 1, false, 4)) {
    Serial.println("[WiFi] softAP start failed");
    while (true) delay(1000);
  }
  const IPAddress ip = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", ip);
  setupRoutes();
  server.begin();
  Serial.printf("[WiFi] SSID=%s password=%s IP=%s\n", AP_SSID, AP_PASSWORD, ip.toString().c_str());
  Serial.println("[Web] open http://192.168.4.1/");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  delay(1);
}

