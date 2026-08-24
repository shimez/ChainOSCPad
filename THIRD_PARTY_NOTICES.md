# Third-Party Notices

ChainOSCPadの独自ソフトウェアとドキュメントは、ルートの[MIT License](LICENSE)で提供されます。
ビルド環境および生成されるファームウェアでは、以下の第三者ソフトウェアを使用します。
各コンポーネントには、それぞれのライセンスが適用されます。

以下は、PlatformIOで本文書を整備した時点の解決結果です。Releaseに対応する正確な依存関係は、対象タグをチェックアウトして確認してください。

## ビルド基盤

| Component | Version | License | Source |
| --- | --- | --- | --- |
| Arduino-ESP32 | 2.0.17 | LGPL-2.1および各構成要素のライセンス | <https://github.com/espressif/arduino-esp32/tree/2.0.17> |

Arduino-ESP32およびその構成要素の対応ソース、著作権表示、ライセンス本文は、上記のバージョン固定リンクを参照してください。

## ライブラリ

| Component | Resolved version / Constraint | License | Copyright notice | Source |
| --- | --- | --- | --- | --- |
| ArduinoOSC | 0.6.0 | MIT | Copyright (c) 2017 Hideaki Tai | <https://github.com/hideakitai/ArduinoOSC> |
| ArduinoJson | 7.4.3 / `^7.4.2` | MIT | Copyright © 2014-2026 Benoit BLANCHON | <https://github.com/bblanchon/ArduinoJson> |
| ArxContainer | 0.7.0（ArduinoOSCの依存関係） | MIT | Copyright (c) 2019 Hideaki Tai | <https://github.com/hideakitai/ArxContainer> |
| ArxSmartPtr | 0.3.0（ArduinoOSCの依存関係） | MIT | Copyright (c) 2020 Hideaki Tai | <https://github.com/hideakitai/ArxSmartPtr> |
| ArxTypeTraits | 0.3.2（ArduinoOSCの依存関係） | MIT | Copyright (c) 2020 Hideaki Tai | <https://github.com/hideakitai/ArxTypeTraits> |
| DebugLog | 0.8.4（ArduinoOSCの依存関係） | MIT | Copyright (c) 2019 Hideaki Tai | <https://github.com/hideakitai/DebugLog> |

第三者コンポーネントの著作権は、それぞれの著作権者に帰属します。PlatformIOとesptoolはビルドおよび書き込み用ツールであり、ChainOSCPad独自部分としてライセンスされるものではありません。
