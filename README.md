# ChainOSCPad

XIAO ESP32S3、3列×4行キーマトリクス、ロータリーエンコーダーを使う
Wi-Fi OSCコントローラーです。

## Version 0.4.1

- 初回起動・Wi-Fi接続失敗時の`ChainOSCPad-Setup` APモード
- ブラウザーからWi-Fi認証情報とOSC送信先を設定
- Wi-Fi／OSC送信先をESP32-S3のNVSへ保存
- `http://chainoscpad.local/`から設定画面へアクセス
- 設定リセット
- Arduino IDE／PlatformIO共通ソース
- Key 1～12のOSC Address、Press／Release値、Float／Int／String型を設定
- Encoder回転のAbsolute／Increment、入出力範囲、出力型を設定
- Encoder ClickのPress／Release複数メッセージとSequenceを設定
- 入力設定をLittleFSへ保存
- 全体設定をバージョン付きJSONでエクスポート／インポート（Wi-Fi認証情報を除外）
- Key／EncoderプリセットをChainOSC共通JSON形式でエクスポート／インポート
- Web UIの日本語／英語切り替えと選択言語の保存

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
5. 使用する2.4 GHz Wi-FiのSSID／パスワードとOSC送信先を保存します。
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

各デバイスカードの`…`メニューからプリセットをJSONで入出力できます。ネットワーク
画面の「設定のバックアップと復元」では、OSC送信先、UI言語、12 Key、Encoderを
まとめて入出力します。全体設定JSONにWi-FiのSSIDとパスワードは含まれません。
同じ`…`メニューから、選択したKeyまたはEncoderだけを初期設定へ戻せます。

## 配線

| XIAO | GPIO | 接続先 |
|---|---:|---|
| D0 | 1 | ROW0（SW1～SW3） |
| D1 | 2 | ROW1（SW4～SW6） |
| D2 | 3 | ROW2（SW7～SW9） |
| D3 | 4 | ROW3（SW10～SW12） |
| D4 | 5 | COL0（SW1、SW4、SW7、SW10） |
| D5 | 6 | COL1（SW2、SW5、SW8、SW11） |
| D6 | 43 | COL2（SW3、SW6、SW9、SW12） |
| D7 | 44 | Encoder A |
| D8 | 7 | Encoder B |
| D9 | 8 | Encoder Push |
| D10 | 9 | Spare（未使用） |

エンコーダーの共通端子とPushの反対側はGNDへ接続します。A、B、Pushには
XIAO ESP32S3の内部プルアップを使用します。

### マトリクスのダイオード

正式版では各スイッチへダイオードを1本ずつ入れてください。このファームウェアは、
ROWを順番にLOWへ駆動してCOLをプルアップ入力として読みます。この走査方向の場合は、
各ダイオードを `COL ->| ROW`、すなわちアノードをCOL側、カソード（帯側）をROW側に
そろえます。

ブレッドボード版をダイオードなしで試すことはできますが、特定の3キー以上を同時に
押すと、押していないキーも押されたように見えるゴーストが発生し得ます。

## OSC初期設定

| 入力 | OSC Address | 型・値 |
|---|---|---|
| Key 1～12 | `/chainoscpad/key/1` ～ `/chainoscpad/key/12` | Float: Press `1.0` / Release `0.0` |
| Encoder回転 | `/avatar/parameters/Encoder` | Float: `0.0`～`0.95`、20段階で循環 |
| Encoder Push | `/avatar/parameters/EncoderClick` | Float: Press `1.0` / Release `0.0` |

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
4. ボードを`XIAO_ESP32S3`または`Seeed XIAO ESP32S3`に設定します。
5. `USB CDC On Boot`を`Enabled`にします。
6. XIAOのポートを選び、検証／書き込みを実行します。
7. シリアルモニターを115200 bpsで開きます。

ボード名が一覧にない場合は、Seeed Studio WikiのXIAO ESP32S3 Arduino IDE導入手順に
従ってボードパッケージを更新してください。

## PlatformIO

1. VS CodeとPlatformIOでこのフォルダーを開きます。
2. XIAO ESP32S3をUSB接続します。
3. ビルドして書き込みます。

```powershell
pio run
pio run --target upload
pio device monitor
```

シリアルモニターは115200 bpsです。OSC受信機とChainOSCPadは同じLANへ接続して
ください。ESP32-S3のWi-Fiは2.4 GHz帯を使用します。

## 調整項目

1クリックで値が変化しない、または2クリック分変化する場合は、`include/config.h`の
`ENCODER_TRANSITIONS_PER_DETENT`を`4`から`2`または`1`へ変更します。

回転方向が逆の場合は、D7とD8を入れ替えるのが簡単です。
