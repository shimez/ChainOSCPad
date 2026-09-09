# ChainOSCPad v1.1.0

ChainOSCPad v1.1.0では、Chain EncoderのDevice Preset v2対応と、既存設定を維持した明示的な移行フローを追加しました。KeyのDevice Preset v1適合性とWebUIも改善しています。

## 主な変更

- Encoderの回転量／回転方向／プッシュ設定をDevice Preset v2でImport／Export
- Device Preset v1 Encoder Presetの互換Import
- Legacy Encoder設定からv2設定への、内容を確認・編集できる明示的な移行
- EncoderのUIDに基づくruntime状態の継続
- Device Preset v1 Keyの検証と、不正Import時に既存設定を変更しない処理の強化
- Key／Encoder WebUIのレイアウト、状態表示、レスポンシブ表示の改善
- WebUIのDevice cardへ製品全体で連番となるDevice番号を表示

## Device Preset互換性

- KeyはDevice Preset v1が正規形式です。KeyにMigrationは不要です。
- EncoderはDevice Preset v1とv2をImportできます。
- 有効なEncoder v1 Presetは、動作を変えずに移行できる場合のみv2として取り込みます。
- v2へ動作を保ったまま変換できない有効なEncoder v1 Presetは、Legacy設定として取り込みます。
- Legacy Encoder設定は通常の保存ではv2へ変換されず、Device Preset v1としてExportされます。
- WebUIで明示的に移行し、検証と保存が成功した場合にのみv2設定になります。

## 更新時の注意

ストレージを消去しない通常のファームウェア更新では、既存設定を維持する設計です。Factory resetは必要ありません。

Web Installerで消去を伴うインストールを選ぶ場合は、保存済みのWi-Fi、OSC送信先、入力設定が失われる可能性があります。事前にWebUIから全体設定と必要なDevice PresetをExportしてください。全体設定のExportにはWi-Fi認証情報が含まれないため、消去後はWi-Fiを再設定してください。

## 対応ハードウェア

- Seeed Studio XIAO ESP32S3
- Seeed Studio XIAO ESP32C3
- Seeed Studio XIAO ESP32C5
- Seeed Studio XIAO ESP32C6
