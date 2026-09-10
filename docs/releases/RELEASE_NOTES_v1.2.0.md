# ChainOSCPad v1.2.0

## Highlights

XIAO D10へ接続した単色LEDによるStatus / Activity表示に対応しました。

## 新機能

- AP Mode、Wi-Fi接続中、Wi-Fi接続完了をLEDパターンで表示
- KeyおよびEncoderのOSC送信処理を短時間の消灯で表示
- XIAO ESP32S3／ESP32C3／ESP32C5／ESP32C6のD10 LEDに対応

## LEDの動作

- AP Mode：ゆっくり点滅
- STA Wi-Fi接続中：2回点滅
- Wi-Fi接続完了：点灯
- OSC Activity：短時間消灯

Wi-Fi切断などによりOSC送信処理がスキップされた場合、Activity表示は行いません。

## 対応ハードウェア

- XIAO ESP32S3
- XIAO ESP32C3
- XIAO ESP32C5
- XIAO ESP32C6

## 注意事項

OSC Activityは、ChainOSCPadの現在のOSC送信経路が送信処理を完了したことを示します。
UDP配送の成功、受信側からのACK、受信側での処理完了を示すものではありません。

LED極性はActive HIGHで実機確認済みです。LED-01～LED-12のPhysical Human AcceptanceをすべてPASSしています。

Device Preset、Migration、Validation、OSC Message、Sequence、Encoder、ネットワークおよびWebUIの既存semanticsに変更はありません。
