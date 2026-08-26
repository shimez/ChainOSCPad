---
layout: default
title: ChainOSCPad 日本語ユーザーガイド
permalink: /user-guide/
---

# ChainOSCPad 日本語ユーザーガイド

[English version](../en/user-guide/)

ChainOSCPadは12個のKeyと、回転・クリック対応のEncoderからOSCを送信するデバイスです。XIAO ESP32S3とXIAO ESP32C6に対応します。

> ChainOSCPadは個人が開発する非公式プロジェクトであり、Seeed Studioその他のハードウェアメーカーによる公式製品ではありません。

## 1. 初回のWi-Fi設定

1. ChainOSCPadをUSB給電します。
2. SSID`ChainOSCPad-Setup`へ接続します。
3. パスワード`12345678`を入力します。
4. キャプティブポータルが開かない場合は`http://192.168.4.1/`を開きます。
5. 2.4 GHz帯Wi-FiのSSIDとパスワードを保存します。

キャプティブポータルではWi-Fi設定だけを行います。

## 2. 通常の設定画面

同じネットワーク上のブラウザーで`http://chainoscpad.local/`を開きます。接続できない場合はシリアルログに表示されるIPアドレスを使用してください。

設定画面には言語、システム情報、Wi-Fi、バックアップと復元、OSC送信先、Key 1～12、Encoderの順に表示されます。変更後は画面に追従する保存ボタンで一括保存します。

## 3. OSC送信先

- IPアドレス：OSC受信機のIPv4アドレス
- UDPポート：`1`～`65535`

Wi-Fi切断中はOSCを送信しません。Sequenceは送信成功時だけ次の値へ進みます。

## 4. KeyとEncoder Click

### Press / Release

PressとReleaseを合わせて最大8メッセージまで設定できます。各メッセージにはOSC Address、型（Float／Int／String）、値を指定できます。

### Sequence

押すたびにStartからStepずつ値を進め、Endを越えるとStartへ戻ります。

- Stepは`0`にできません。
- StartがEndより小さい場合、Stepは正数にします。
- StartがEndより大きい場合、Stepは負数にします。

## 5. Encoder回転

Encoderの時計回り／反時計回りに送信するOSC Address、型、値を設定できます。設定画面のEncoderカードから編集します。

## 6. プリセットとバックアップ

各Key／Encoderの`…`メニューからプリセットをJSONでエクスポート／インポートできます。全体設定は「設定のバックアップと復元」からJSONで保存・復元できます。Wi-Fi認証情報は全体設定JSONに含まれません。

ファームウェアの再インストールや消去前には、必要な設定をエクスポートしてください。

## 7. 設定の初期化

各カードの`…`メニューから、そのKeyまたはEncoderの設定を初期化できます。Wi-Fi設定はWi-Fiセクションの削除ボタンから消去できます。

[ChainOSCPadポータルへ戻る](../)
