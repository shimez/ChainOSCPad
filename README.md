# ChainOSCPad

XIAO ESP32S3／ESP32C3／ESP32C5／ESP32C6に対応した、3列×4行キーマトリクスとロータリーエンコーダーを搭載するWi-Fi OSCコントローラーです。Key、Encoder、Wi-Fi、OSC送信先をブラウザーから設定し、VRChatなどOSCを受信できるアプリケーションで利用できます。

このプロジェクトのソフトウェア、Webサイト、ドキュメントは、OpenAI Codexとの協働により制作されています。

## 現在のバージョン

### v1.2.0

- XIAO D10のStatus / Activity LEDに対応
- AP Mode、Wi-Fi接続中、Wi-Fi接続完了、OSC Activityの状態表示を追加
- KeyおよびEncoderのOSC送信処理とLED表示を連携

詳細な変更履歴は[CHANGELOG.md](CHANGELOG.md)を参照してください。

## 主な機能

- XIAO ESP32S3／ESP32C3／ESP32C5／ESP32C6対応
- 3列×4行、12キーのマトリクス入力
- ロータリーエンコーダー回転およびプッシュ入力
- KeyのPress / ReleaseおよびSequence
- Encoder Rotationの回転量／回転方向モード
- Encoder PushのPress / ReleaseおよびSequence
- OSC送信先、OSC Address、型、値の設定
- Wi-Fi認証情報、OSC送信先、UI言語、入力設定のLittleFS保存
- 全体設定のJSONバックアップ／復元
- Device Presetのエクスポート／インポート
- Device Preset v1／v2の検証と日英Error Registry
- Legacy Encoder設定とV2 Encoder設定の区別
- AP Modeとキャプティブポータル
- 日本語／英語Web UI
- XIAO D10のStatus / Activity LED
- Arduino IDE、PlatformIO、Web Installer対応

## Documentation

- [日本語ユーザーガイド](docs/USER_GUIDE_ja.md)
- [English User Guide](docs/USER_GUIDE_en.md)
- [日本語Quick Start](docs/QUICK_START_ja.md)
- [English Quick Start](docs/QUICK_START_en.md)
- [Web Installer](https://shimez.github.io/ChainOSCPad/installer/)
- [実機確認項目](TESTING.md)
- [Status / Activity LED Human Acceptance](docs/testing/STATUS_ACTIVITY_LED_HUMAN_ACCEPTANCE.md)
- [変更履歴](CHANGELOG.md)
- [ChainOSC共通仕様](https://github.com/shimez/ChainOSC)

## Device Preset対応

| Device Type | Preset v1 | Preset v2 | 備考 |
|---|---|---|---|
| Key | 対応 | 非対応 | Device Preset v1がmaintained canonical formatです。Migrationは不要です。 |
| Encoder | 対応 | 対応 | v1互換Importとv2 Import／Exportに対応します。 |

ChainOSCPadはChainOSCシリーズ共通の`ChainOSC-device-preset`形式に対応します。詳細な仕様、JSON Schema、fixture、Error Registryは[ChainOSC共通仕様](https://github.com/shimez/ChainOSC)を参照してください。

有効なEncoder v1 Presetは、動作を変えずに移行できる場合のみv2として取り込みます。それ以外はLegacy設定として取り込みます。Legacy設定は通常保存で暗黙にv2へ変換されず、明示的な移行の検証・保存成功時のみv2になります。キャンセルまたは保存失敗時はLegacy設定を維持します。

## Status / Activity LED

- AP Mode：ゆっくり点滅
- Wi-Fi接続中：2回点滅
- Wi-Fi接続完了：点灯
- OSC Activity：短時間消灯

OSC ActivityはChainOSCPad内部のOSC送信経路の処理完了を示します。UDP配送や受信側での受信・処理完了を保証する表示ではありません。

## 初回設定

1. ファームウェアを書き込みます。
2. `ChainOSCPad-Setup`へ接続し、パスワード`12345678`を入力します。
3. 表示されない場合は`http://192.168.4.1/`を開きます。
4. Wi-FiのSSID／パスワードとOSC送信先を保存します。
5. 再起動後、`http://chainoscpad.local/`を開きます。

ESP32C5は2.4 GHz／5 GHz、ESP32S3／C3／C6は2.4 GHzに対応します。設定ページには認証機能がないため、信頼できるローカルネットワークで使用してください。

## 入力設定

Key 1～12、Encoder、OSC送信先、JSONバックアップと復元を1つの設定画面で設定できます。Press／Releaseは合計最大8メッセージ、SequenceはOSC Address、Start、End、Step、型を設定します。Encoderは回転量／回転方向、範囲、ループ／停止、出力型、Push設定を持ちます。設定は検証後にLittleFSへ保存され、保存失敗時は既存設定を維持します。

## 配線

| XIAO | S3 | C6 | C3 | C5 | 接続先 |
|---|---:|---:|---:|---:|---|
| D0 | 1 | 0 | 2 | 1 | ROW0 |
| D1 | 2 | 1 | 3 | 0 | ROW1 |
| D2 | 3 | 2 | 4 | 25 | ROW2 |
| D3 | 4 | 21 | 5 | 7 | ROW3 |
| D4 | 5 | 22 | 6 | 23 | COL0 |
| D5 | 6 | 23 | 7 | 24 | COL1 |
| D6 | 43 | 16 | 21 | 11 | COL2 |
| D7 | 44 | 17 | 20 | 12 | Encoder A |
| D8 | 7 | 19 | 8 | 8 | Encoder B |
| D9 | 8 | 20 | 9 | 9 | Encoder Push |
| D10 | 9 | 18 | 10 | 10 | Status / Activity LED |

エンコーダーの共通端子とPushの反対側はGNDへ接続します。A、B、Pushには内部プルアップを使用します。

D10とStatus / Activity LEDの間には、LEDと直列に適切な電流制限抵抗を入れてください。抵抗値は使用するLEDまたはLED内蔵エンコーダーの仕様に合わせて選定してください。D10へ抵抗なしで直接接続することは推奨しません。

正式版では各スイッチへダイオードを1本ずつ入れてください。このファームウェアは、ROWを順番にLOWへ駆動してCOLをプルアップ入力として読みます。この走査方向では、各ダイオードを`COL ->| ROW`、すなわちアノードをCOL側、カソード（帯側）をROW側にそろえます。

ブレッドボード版をダイオードなしで試すことはできますが、特定の複数キーを同時に押すと、押していないキーも押されたように見えるghostingが発生し得ます。

## OSC初期設定

| 入力 | OSC Address | 型・値 |
|---|---|---|
| Key 1～12 | `/chainoscpad/key/1` ～ `/chainoscpad/key/12` | Float: Press `1.0` / Release `0.0` |
| Encoder回転 | `/avatar/parameters/Encoder` | Float: `0.0`～`1.0` |
| Encoder Push | `/avatar/parameters/EncoderPush` | Float: Press `1.0` / Release `0.0` |

## 旧固定設定ファイル

現在は通常`include/secrets.h`を作成する必要はありません。旧設定例は参照用として残しています。`secrets.h`は`.gitignore`の対象であり、Wi-FiパスワードをGitHubへpushしないでください。

## Arduino IDE

Arduino IDEで`ChainOSCPad.ino`を開き、使用するXIAO ESP32S3／C3／C5／C6用ボードを選択します。`USB CDC On Boot`は`Enabled`にし、シリアルモニターは`115200 bps`で開きます。

## PlatformIO

```powershell
pio run
pio run -e xiao_esp32s3
pio run -e xiao_esp32c3
pio run -e xiao_esp32c5
pio run -e xiao_esp32c6
pio run -e xiao_esp32s3 --target upload
pio run -e xiao_esp32c3 --target upload
pio run -e xiao_esp32c5 --target upload
pio run -e xiao_esp32c6 --target upload
pio device monitor --baud 115200
```

## 自動ビルドとWeb Installer

`vX.Y.Z`タグのpushまたは手動実行で、GitHub ActionsがS3／C3／C5／C6をビルドし、タグ実行時はDraft Releaseを作成します。Draft Releaseを公開するとGitHub PagesとWeb Installerへ反映されます。

- [ChainOSCPad製品ポータル](https://shimez.github.io/ChainOSCPad/)
- [Web Installer](https://shimez.github.io/ChainOSCPad/installer/)

ローカルテスト:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test_web_installer.ps1
```

## 調整項目

1クリックで値が変化しない、または2クリック分変化する場合は、`include/config.h`の`ENCODER_TRANSITIONS_PER_DETENT`を`4`から`2`または`1`へ変更します。

回転方向が逆の場合は、D7とD8を入れ替えるのが簡単です。

### ESP32C5 Encoder診断

調査時は`xiao_esp32c5`環境のbuild flagsへ`-DCHAINOSCPAD_ENCODER_DIAGNOSTICS=1`を追加します。115200 bpsのシリアルモニターを開き、時計回り1クリック、反時計回り1クリック、時計回り10クリック、反時計回り10クリック、速い回転の順に試してください。回転中の状態はRAMへ記録され、200 ms静止するとまとめてシリアルへ出力されるため、ログ出力自体による取りこぼしを抑えています。

- `AB`：D7／D8の直前と現在の論理状態
- `delta`：正常遷移の方向（`1`または`-1`）。`0`は同時変化などの無効遷移
- `acc`：1クリック判定までの累積遷移数。現在の判定値は`4`
- `invalid`：一連の回転中に検出した無効遷移数
- `dropped`：診断用RAMバッファへ収まらなかった状態遷移数
- `max-gap`：一連の回転中で最長のEncoderポーリング間隔
- `network`／`matrix`／`encoder`：各処理ブロックの最大実行時間

1クリックで4つの同方向遷移が揃わない場合は、エンコーダーの遷移数または接点の問題を疑います。速い回転時だけ`max-gap`が大きくなって欠落する場合は、処理待ちによるポーリングの取りこぼしを疑います。公開用ビルドでは診断フラグを設定しません。

ESP32C5が使用するArduino-ESP32 3.3.7の`WebServer::handleClient()`は、待機中のHTTPクライアントがない場合に内部で`delay(1)`を呼びます。この環境では約50 msの停止になりEncoderの状態遷移を取りこぼすため、C5のみ`enableDelay(false)`でWebServerのidle delayを無効化します。メインループでも明示的な待機は行いません。

また、C5のみROWピンを起動時に一度だけオープンドレイン出力へ設定します。走査中はLOWとハイインピーダンスを切り替え、時間のかかる`pinMode()`を毎回呼ばない構成です。S3／C3／C6は従来のINPUT／OUTPUT切り替え方式を維持します。

## ライセンス

ChainOSCPad独自のソフトウェア、Web UI、ドキュメントは[MIT License](LICENSE)で提供されます。第三者ソフトウェアのライセンス、著作権表示、対応ソースは[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)を参照してください。
