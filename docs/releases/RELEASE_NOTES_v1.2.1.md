# ChainOSCPad v1.2.1

v1.2.1は、EncoderのLegacy設定およびv2移行候補の表示を改善するPATCHリリースです。

## 変更内容

- Legacy Encoderの回転設定を3列レイアウトへ整理
- Legacyの端点動作を「範囲をループする」チェックボックスで表示
- 増分モードではAbsolute専用項目を非表示にし、デスクトップ表示で他の項目の位置を維持
- 増分設定から生成したv2回転方向候補では、回転量モード専用の「範囲ステップ数」と「回転方向」を非表示化

## 互換性

EncoderのLegacy／v2データ構造、Device Preset、保存形式、runtime semantics、OSC送信処理に変更はありません。
