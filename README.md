# ChainOSCPad

このプロジェクトのソフトウェア、Webサイト、ドキュメントは、OpenAI Codexとの協働により制作されています。

This project's software, website, and documentation are created in collaboration with OpenAI Codex.

XIAO ESP32S3／ESP32C3／ESP32C5／ESP32C6に対応した、3列×4行キーマトリクスと
ロータリーエンコーダーを搭載するWi-Fi OSCコントローラーです。

## Version 0.8.1

- 初回起動・Wi-Fi接続失敗時の`ChainOSCPad-Setup` APモード
- ブラウザーからWi-Fi認証情報とOSC送信先を設定
- Wi-Fi／OSC送信先をESP32のNVSへ保存
- `http://chainoscpad.local/`から設定画面へアクセス
- 設定リセット
- Arduino IDE／PlatformIO共通ソース
- Key 1～12のOSC Address、Press／Release値、Float／Int／String型を設定
- Encoder回転のAbsolute／Increment、入出力範囲、出力型を設定
- Encoder ClickのPress／Release複数メッセージとSequenceを設定
- 入力設定をLittleFSへ保存
- 全体設定をバージョン付きJSONでエクスポート／インポート（Wi-Fi認証情報を除外）
- Key／EncoderプリセットをChainOSC共通JSON形式でエクスポート／インポート
- Device Preset Import Error Registry v1に準拠したプリセット検証と日英エラーメッセージ
- Web UIの日本語／英語切り替えと選択言語の保存
- Web UIのシステム欄にXIAO ESP32S3／ESP32C3／ESP32C5／ESP32C6のモデル名を表示

変更履歴は[CHANGELOG.md](CHANGELOG.md)を参照してください。
実機確認項目は[TESTING.md](TESTING.md)にまとめています。

## 実装済み

- 12キーのPress／Release OSC送信
- 全キーの独立デバウンス
- 複数キー入力を保持できるマトリクススキャン
- ロータリーエンコーダーのGray-codeデコード
- ChainOSC既定仕様に合わせたAbsolute値の循環・Float送信
- エンコーダープッシュのPress／Release送信
- Wi-Fi切断時の再接続
- USBシリアルへの入力・OSC送信ログ

## 初回設定

1. ファームウェアを書き込みます。
2. PCまたはスマートフォンからWi-Fi AP`ChainOSCPad-Setup`へ接続します。
3. パスワード`12345678`を入力します。
4. 設定画面が自動表示されない場合は`http://192.168.4.1/`を開きます。
5. 使用するWi-FiのSSID／パスワードとOSC送信先を保存します。ESP32C5は
   2.4 GHz／5 GHz、ESP32S3／C3／C6は2.4 GHzに対応します。
6. 自動再起動後、同じLANから`http://chainoscpad.local/`を開けます。

保存済みWi-Fiへ起動後15秒以内に接続できない場合も、設定APモードへ移行します。
設定ページには認証機能がないため、信頼できるローカルネットワークで使用してください。

## 入力設定

Wi-Fi接続後に`http://chainoscpad.local/`を開きます。Wi-Fi／OSC送信先、JSONの
バックアップと復元、Key 1～12、Encoderは1つの設定画面に表示されます。

- Key 1～12：`Press / Release`または`Sequence`を選択
- Press／Release：合計最大8メッセージ、0件可。OSC Address、型、値、送信順を設定
- Sequence：OSC Address、Start、End、Step、Float／Int／String型を設定
- Encoder Rotation：OSC Address、Absolute／Increment、入力範囲、増分倍率、
  出力範囲、Float／Int／String型
- Encoder Click：Keyと同じPress／Release最大8メッセージとSequence

全項目を検証してからLittleFSへ保存し、その場で動作へ反映します。Float／Intで
数値として解釈できない値や、`/`で始まらないOSC Addressは拒否されます。
OSC送信先、Key 1～12、Encoderは、画面下部の「すべての設定を保存」でまとめて
保存されます。

各デバイスカードの`…`メニューからプリセットをJSONで入出力できます。ネットワーク
画面の「設定のバックアップと復元」では、OSC送信先、UI言語、12 Key、Encoderを
まとめて入出力します。全体設定JSONにWi-FiのSSIDとパスワードは含まれません。
同じ`…`メニューから、選択したKeyまたはEncoderだけを初期設定へ戻せます。

## 配線

| XIAO | ESP32S3 GPIO | ESP32C6 GPIO | ESP32C3 GPIO | ESP32C5 GPIO | 接続先                      |
| ---- | -----------: | -----------: | -----------: | -----------: | --------------------------- |
| D0   |            1 |            0 |            2 |            1 | ROW0（SW1～SW3）            |
| D1   |            2 |            1 |            3 |            0 | ROW1（SW4～SW6）            |
| D2   |            3 |            2 |            4 |           25 | ROW2（SW7～SW9）            |
| D3   |            4 |           21 |            5 |            7 | ROW3（SW10～SW12）          |
| D4   |            5 |           22 |            6 |           23 | COL0（SW1、SW4、SW7、SW10） |
| D5   |            6 |           23 |            7 |           24 | COL1（SW2、SW5、SW8、SW11） |
| D6   |           43 |           16 |           21 |           11 | COL2（SW3、SW6、SW9、SW12） |
| D7   |           44 |           17 |           20 |           12 | Encoder A                   |
| D8   |            7 |           19 |            8 |            8 | Encoder B                   |
| D9   |            8 |           20 |            9 |            9 | Encoder Push                |
| D10  |            9 |           18 |           10 |           10 | Spare（未使用）             |

エンコーダーの共通端子とPushの反対側はGNDへ接続します。A、B、Pushには
各対応XIAOの内部プルアップを使用します。

### マトリクスのダイオード

正式版では各スイッチへダイオードを1本ずつ入れてください。このファームウェアは、
ROWを順番にLOWへ駆動してCOLをプルアップ入力として読みます。この走査方向の場合は、
各ダイオードを `COL ->| ROW`、すなわちアノードをCOL側、カソード（帯側）をROW側に
そろえます。

ブレッドボード版をダイオードなしで試すことはできますが、特定の3キー以上を同時に
押すと、押していないキーも押されたように見えるゴーストが発生し得ます。

## OSC初期設定

| 入力         | OSC Address                                   | 型・値                             |
| ------------ | --------------------------------------------- | ---------------------------------- |
| Key 1～12    | `/chainoscpad/key/1` ～ `/chainoscpad/key/12` | Float: Press `1.0` / Release `0.0` |
| Encoder回転  | `/avatar/parameters/Encoder`                  | Float: `0.0`～`0.95`、20段階で循環 |
| Encoder Push | `/avatar/parameters/EncoderClick`             | Float: Press `1.0` / Release `0.0` |

エンコーダーはChainOSCの既定値と同じく、Absolute入力範囲`0～20`を出力範囲
`0～1`へマッピングし、20で0へ戻します。したがって整数ステップで実際に送信される値は
`0.00, 0.05, ... 0.95`です。

## 旧固定設定ファイル

Version 0.2.0ではWi-FiとOSC送信先をブラウザーから設定するため、通常は
`include/secrets.h`を作成する必要はありません。旧Version 0.1.0用の設定例は、
参照用として残しています。

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `OSC_TARGET_HOST`
- 必要なら`OSC_TARGET_PORT`

`secrets.h`は引き続き`.gitignore`の対象です。Wi-FiパスワードをGitHubへpushしないで
ください。

## Arduino IDE

1. Arduino IDEのBoards ManagerでEspressif Systemsの`esp32`ボードパッケージを
   インストールします。
2. Library Managerで`ArduinoOSC`と`ArduinoJson`をインストールします。
3. ルートの`ChainOSCPad.ino`をArduino IDEで開きます。
4. 使用するマイコンに合わせて`XIAO_ESP32S3`／`Seeed XIAO ESP32S3`または
   `XIAO_ESP32C6`／`Seeed Studio XIAO ESP32C6`に設定します。
5. `USB CDC On Boot`を`Enabled`にします。
6. XIAOのポートを選び、検証／書き込みを実行します。
7. シリアルモニターを115200 bpsで開きます。

ボード名が一覧にない場合は、Seeed Studio Wikiの各XIAO Arduino IDE導入手順に
従ってボードパッケージを更新してください。ESP32C6ではArduino ESP32 3.x系を
使用してください。

## PlatformIO

1. VS CodeとPlatformIOでこのフォルダーを開きます。
2. 使用するXIAOをUSB接続します。
3. 対象環境を指定してビルド・書き込みします。対象を省略した`pio run`ではS3とC6を
   両方ビルドします。

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

環境別のファームウェアは`.pio/build/xiao_esp32s3/`、`.pio/build/xiao_esp32c3/`、
`.pio/build/xiao_esp32c5/`、`.pio/build/xiao_esp32c6/`に生成されます。

## 自動ビルドとWeb Installer

`vX.Y.Z`タグのpushまたは手動実行で、GitHub Actionsが`xiao_esp32s3`、
`xiao_esp32c3`、`xiao_esp32c5`、`xiao_esp32c6`を個別にビルドします。
各環境の統合書き込み用binとSHA-256を
Actions Artifactとして保存し、タグ実行時は4環境のファイルを添付したDraft Releaseを
自動作成します。内容を確認してDraftを公開してください。

GitHub Pagesの製品ポータル、Web Installer、製品アイコンは`docs/`に格納しています。

Pagesは`docs/`の変更またはReleaseの公開を検知します。manifestと同じバージョンの
公開済みReleaseが存在する場合だけ、Releaseから両環境の統合binを取得して製品ポータルと
Web InstallerをGitHub Pagesへ公開します。Draftの間は現在の公開ページを維持します。
初回のみGitHubリポジトリの`Settings` → `Pages` → `Build and deployment`でSourceを
`GitHub Actions`に設定してください。公開先は通常、
`https://shimez.github.io/ChainOSCPad/`です。ポータルのWeb Installerリンクから
`https://shimez.github.io/ChainOSCPad/installer/`を開き、PC版Google Chromeまたは
Microsoft Edgeで接続したXIAOへ書き込めます。チップに合うS3/C3/C5/C6ファームウェアは
インストーラーが自動選択します。OG／Twitterカードは製品ポータル用です。

C3/C5はPlatformIOとReleaseのビルド対象です。Web Installerへの追加は、実機での
書き込みと基本動作を確認した後に行います。

### Web Installerを公開前にテストする

PowerShellで次を実行すると、S3/C3/C5/C6をビルドして統合binを生成し、manifestとファームウェア構成を検証して、
ローカルWeb Installerを`http://127.0.0.1:8765/installer/`で起動します。

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test_web_installer.ps1
```

すでに両環境をビルド済みなら`-SkipBuild`、ファイル検証だけなら`-CheckOnly`も指定できます。
ChromeまたはEdgeでInstallerを開き、XIAO ESP32S3とESP32C6を1台ずつUSB接続して、
それぞれ書き込みを確認してください。ESP Web ToolsはUSBから取得したチップ種別と
manifestの`ESP32-S3`／`ESP32-C3`／`ESP32-C5`／`ESP32-C6`を照合して対象ファームウェアを選択します。

S3環境は既存動作を維持するためEspressif 32 Platform 6.9.0を使用します。C6環境は
Seeed Studio公式PlatformIO platformのWindowsで検証したリビジョンを使用します。

シリアルモニターは115200 bpsです。OSC受信機とChainOSCPadは同じLANへ接続して
ください。ESP32C5は2.4 GHz／5 GHz、ESP32S3／C3／C6は2.4 GHz帯を
使用します。

## 調整項目

1クリックで値が変化しない、または2クリック分変化する場合は、`include/config.h`の
`ENCODER_TRANSITIONS_PER_DETENT`を`4`から`2`または`1`へ変更します。

回転方向が逆の場合は、D7とD8を入れ替えるのが簡単です。

### ESP32C5 Encoder診断

調査時は`xiao_esp32c5`環境のbuild flagsへ
`-DCHAINOSCPAD_ENCODER_DIAGNOSTICS=1`を追加します。115200 bpsの
シリアルモニターを開き、時計回り1クリック、
反時計回り1クリック、時計回り10クリック、反時計回り10クリック、速い回転の
順に試してください。回転中の状態はRAMへ記録され、200 ms静止するとまとめて
シリアルへ出力されるため、ログ出力自体による取りこぼしを抑えています。

- `AB`：D7／D8の直前と現在の論理状態
- `delta`：正常遷移の方向（`1`または`-1`）。`0`は同時変化などの無効遷移
- `acc`：1クリック判定までの累積遷移数。現在の判定値は`4`
- `invalid`：一連の回転中に検出した無効遷移数
- `dropped`：診断用RAMバッファへ収まらなかった状態遷移数
- `max-gap`：一連の回転中で最長のEncoderポーリング間隔
- `network`／`matrix`／`encoder`：各処理ブロックの最大実行時間

1クリックで4つの同方向遷移が揃わない場合は、エンコーダーの遷移数または接点の
問題を疑います。速い回転時だけ`max-gap`が大きくなって欠落する場合は、処理待ちに
よるポーリングの取りこぼしを疑います。公開用ビルドでは診断フラグを設定しません。

ESP32C5が使用するArduino-ESP32 3.3.7の`WebServer::handleClient()`は、待機中の
HTTPクライアントがない場合に内部で`delay(1)`を呼びます。この環境では約50 msの
停止になりEncoderの状態遷移を取りこぼすため、C5のみ`enableDelay(false)`で
WebServerのidle delayを無効化します。メインループでも明示的な待機は行いません。

また、C5のみROWピンを起動時に一度だけオープンドレイン出力へ設定します。走査中は
LOWとハイインピーダンスを切り替え、時間のかかる`pinMode()`を毎回呼ばない構成です。
S3／C3／C6は従来のINPUT／OUTPUT切り替え方式を維持します。

## ライセンス

特に明記がない限り、現在このリポジトリで公開しているChainOSCPad独自の
ソフトウェア、Web UIおよびドキュメントは[MIT License](LICENSE)で提供されます。

ビルド環境および生成されるファームウェアでは、別のライセンスで提供される
第三者ソフトウェアを使用します。対象コンポーネント、バージョン、著作権表示、
ライセンスおよび対応ソースは[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)を
参照してください。

将来公開する回路図、PCBレイアウト、Gerber、BOM、筐体データなどの
ハードウェア設計資料には、別途明記するライセンスが適用される場合があります。
