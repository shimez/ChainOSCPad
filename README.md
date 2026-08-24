# ChainOSCPad 固定設定・動作確認版

XIAO ESP32S3、3列×4行キーマトリクス、ロータリーエンコーダーを使う
ChainOSCPadの最初のハードウェア確認用ファームウェアです。設定用Web UIやNVS保存は、
この版には含まれません。

## 実装済み

- 12キーのPress／Release OSC送信
- 全キーの独立デバウンス
- 複数キー入力を保持できるマトリクススキャン
- ロータリーエンコーダーのGray-codeデコード
- ChainOSC既定仕様に合わせたAbsolute値の循環・Float送信
- エンコーダープッシュのPress／Release送信
- Wi-Fi切断時の再接続
- USBシリアルへの入力・OSC送信ログ

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

## 固定OSC設定

| 入力 | OSC Address | 型・値 |
|---|---|---|
| Key 1～12 | `/chainoscpad/key/1` ～ `/chainoscpad/key/12` | Float: Press `1.0` / Release `0.0` |
| Encoder回転 | `/avatar/parameters/Encoder` | Float: `0.0`～`0.95`、20段階で循環 |
| Encoder Push | `/avatar/parameters/EncoderClick` | Float: Press `1.0` / Release `0.0` |

エンコーダーはChainOSCの既定値と同じく、Absolute入力範囲`0～20`を出力範囲
`0～1`へマッピングし、20で0へ戻します。したがって整数ステップで実際に送信される値は
`0.00, 0.05, ... 0.95`です。

## 設定ファイル

`include/secrets.example.h`を同じフォルダーの`secrets.h`へコピーし、次の値を
書き換えます。

   - `WIFI_SSID`
   - `WIFI_PASSWORD`
   - `OSC_TARGET_HOST`
   - 必要なら`OSC_TARGET_PORT`

`secrets.h`は`.gitignore`の対象です。Wi-FiパスワードをGitHubへpushしないでください。
ファイルを作成しなくてもプレースホルダー設定でコンパイルできますが、Wi-Fi接続は
開始されません。

## Arduino IDE

1. Arduino IDEのBoards ManagerでEspressif Systemsの`esp32`ボードパッケージを
   インストールします。
2. Library Managerで`ArduinoOSC`をインストールします。
3. 上記の手順で`include/secrets.h`を作成します。
4. ルートの`ChainOSCPad.ino`をArduino IDEで開きます。
5. ボードを`XIAO_ESP32S3`または`Seeed XIAO ESP32S3`に設定します。
6. `USB CDC On Boot`を`Enabled`にします。
7. XIAOのポートを選び、検証／書き込みを実行します。
8. シリアルモニターを115200 bpsで開きます。

ボード名が一覧にない場合は、Seeed Studio WikiのXIAO ESP32S3 Arduino IDE導入手順に
従ってボードパッケージを更新してください。

## PlatformIO

1. VS CodeとPlatformIOでこのフォルダーを開きます。
2. 上記の手順で`include/secrets.h`を作成します。
3. XIAO ESP32S3をUSB接続します。
4. ビルドして書き込みます。

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
