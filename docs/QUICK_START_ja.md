---
layout: default
title: ChainOSCPad クイックスタート
permalink: /quick-start/
---

# ChainOSCPad クイックスタート

[English version](../en/quick-start/)

このガイドでは、Device Presetを使わずにKeyの設定を手入力し、ChainOSCPadからVRChatへOSCメッセージを送信して動作を確認します。詳しい設定は[日本語ユーザーガイド](../user-guide/)を参照してください。

## 用意するもの

- 組み立て済みのChainOSCPad
- XIAO ESP32S3、ESP32C3、ESP32C5、またはESP32C6
- データ通信対応USB Type-Cケーブル
- 対応するWi-Fi（ESP32C5は2.4 GHz／5 GHz、ESP32S3／C3／C6は2.4 GHz）
- デスクトップ版ChromeまたはEdge
- VRChatを実行するPC

## 1. ファームウェアを書き込む

1. [ChainOSCPad Web Installer](../installer/)をChromeまたはEdgeで開きます。
2. ChainOSCPadをUSBケーブルでPCへ接続します。
3. `Install ChainOSCPad`を押し、XIAOのシリアルポートを選択します。
4. 画面の案内に従ってインストールします。

ポートが表示されない場合は、XIAOのBOOTボタンを押しながらUSBへ接続してください。

## 2. Wi-Fiを設定する

1. SSID `ChainOSCPad-Setup`へ接続します。
2. パスワード`12345678`を入力します。
3. キャプティブポータルが開かない場合は`http://192.168.4.1/`を開きます。
4. 使用するWi-Fiを選択し、パスワードを保存します。ESP32C5は2.4 GHz／
   5 GHz、ESP32S3／C3／C6は2.4 GHzに対応します。

## 3. VRChatでOSCを有効にする

VRChatを起動し、リングメニュー → オプション → OSC → 有効に設定します。

## 4. VRChatを実行しているPCのIPv4アドレスを確認する

WindowsでPowerShellまたはコマンドプロンプトを開き、`ipconfig`を実行します。ChainOSCPadと同じネットワークに接続しているWi-FiまたはEthernetアダプターのIPv4アドレスを確認してください。VPNや仮想アダプターではなく、実際に接続中のアダプターを選びます。

## 5. 設定画面を開く

同じネットワークから`http://chainoscpad.local/`を開きます。開けない場合はシリアルログに表示されるIPアドレスをブラウザーで開いてください。

## 6. OSC送信先を設定する

1. 「OSC送信先の設定」の「IPアドレス」に、VRChatを実行しているPCのIPv4アドレスを入力します。
2. 「UDPポート」に`9000`を入力します。

## 7. KeyにVoice操作を設定する

設定画面の「#1 Key」カード（デフォルトのデバイス名は「Key 1」）で、「押した時」に次の値を入力します。

- OSC Address：`/input/Voice`
- 型：`Int`
- 値：`1`

「離した時」に切り替えて、次の値を入力します。

- OSC Address：`/input/Voice`
- 型：`Int`
- 値：`0`

## 8. 保存して動作を確認する

1. 「すべての設定を保存」を押します。
2. VRChatが起動していてOSCが有効な状態で、「#1 Key」カードのKeyを操作します。
3. VRChatのVoice入力状態が切り替わることを確認します。Voiceが切り替われば、ChainOSCPadからOSCメッセージを送信できています。

ChainOSCPadはVRChat専用ではありません。VRChat以外のOSC対応アプリケーションでも、送信先、OSC Address、型、値をそのアプリケーションに合わせて設定すれば利用できます。よく使う設定の再利用・共有にはDevice Presetを利用できます。詳しくは[日本語ユーザーガイド](../user-guide/)を参照してください。

[ChainOSCPadポータルへ戻る](../)
