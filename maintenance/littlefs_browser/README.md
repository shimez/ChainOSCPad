# ChainOSCPad LittleFS Browser

XIAO ESP32S3上の既存LittleFSを調査するための独立メンテナンスファームです。
ChainOSCPad本体機能は含みません。

## 重要

- `LittleFS.begin(false)`を使用し、マウント失敗時にも自動フォーマットしません。
- PlatformIOの通常の`upload`はアプリ領域だけを書き換え、LittleFS領域を消去しません。
- `Erase Flash`や`uploadfs`は実行しないでください。
- 編集・削除の前に、必要なファイルをDownloadでバックアップしてください。

## ビルドと書き込み

VS Codeでこの`maintenance/littlefs_browser`フォルダーを別ウィンドウとして開きます。
ターミナルから実行する場合：

```powershell
cd C:\Users\ctake\OneDrive\Arduino\ChainOSCPad\maintenance\littlefs_browser
C:\Users\ctake\.platformio\penv\Scripts\platformio.exe run
C:\Users\ctake\.platformio\penv\Scripts\platformio.exe run --target upload
C:\Users\ctake\.platformio\penv\Scripts\platformio.exe device monitor
```

## 接続

- SSID: `ChainOSCPad-FS`
- Password: `chainosc-fs`
- URL: `http://192.168.4.1/`

ブラウザーから一覧表示、Text/Hex表示・編集、ダウンロード、アップロード／置換、削除が可能です。
Web上での編集上限は16 KiBです。大きなファイルはダウンロードしてからアップロードで置換してください。

調査終了後は通常のChainOSCPadファームを書き戻します。通常のファーム書き込みだけではLittleFSは消去されません。
