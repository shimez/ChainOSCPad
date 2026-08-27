---
layout: default
title: ChainOSCPad クイックスタート
permalink: /quick-start/
---

# ChainOSCPad クイックスタート

[English version](../en/quick-start/)

このガイドでは、ファームウェアのインストールからOSC送信の確認までを案内します。詳しい設定は[日本語ユーザーガイド](../user-guide/)を参照してください。

## 用意するもの

- 組み立て済みのChainOSCPad
- XIAO ESP32S3またはXIAO ESP32C6
- データ通信対応USB Type-Cケーブル
- 対応するWi-Fi（ESP32C5は2.4 GHz／5 GHz、ESP32S3／C3／C6は2.4 GHz）
- デスクトップ版ChromeまたはEdge
- OSCを受信するPCまたはアプリケーション

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

## 3. OSC送信先を設定する

同じネットワークから`http://chainoscpad.local/`を開きます。OSC送信先のIPアドレスとUDPポートを入力し、画面下部の保存ボタンで全体を保存します。

## 4. 動作を確認する

KeyまたはEncoderを操作し、設定したOSCアドレスと値が受信側へ届くことを確認します。

[ChainOSCPadポータルへ戻る](../)
