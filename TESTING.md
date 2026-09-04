# Version 1.0.2 実機確認

## Web UI全設定削除

- [ ] Web UI最下段に赤色の「すべての設定を削除」ボタンが表示される
- [ ] キャンセルすると設定が変更されない
- [ ] 確認して実行するとWi-Fi、OSC送信先、UI言語、Key／Encoder設定が削除される
- [ ] 再起動後に設定APで起動し、旧NVS設定が復元されない

## AP Mode全設定削除

- [ ] AP Modeのキャプティブポータル最下段に赤色の「すべての設定を削除」ボタンが表示される
- [ ] キャンセルすると設定が変更されない
- [ ] 確認するとWi-Fi、OSC送信先、UI言語、Key／Encoder設定が削除される
- [ ] 削除後に再起動し、再びAP Modeで起動する

## LittleFS共通設定移行

- [x] 旧バージョンで保存したWi-Fi、OSC送信先、UI言語が更新後の初回起動で維持される
- [x] 初回移行後に`/system/settings.json`が作成され、シリアルへ容量と結果が表示される
- [x] NVSとLittleFSの双方に設定がある場合、有効なLittleFS設定が優先される
- [x] 再起動後もWi-Fi接続、OSC送信先、UI言語が維持される
- [x] Wi-Fi設定の削除後もOSC送信先、UI言語、Key／Encoder設定が維持される
- [x] 全設定の削除後に旧NVS設定が再移行されず、設定APで起動する
- [x] 全体設定JSONのインポート後、OSC送信先とUI言語が再起動後も維持される

## ビルド対象

- [x] PlatformIOの`xiao_esp32s3`環境でビルドできる
- [x] PlatformIOの`xiao_esp32c3`環境でビルドできる
- [x] PlatformIOの`xiao_esp32c5`環境でビルドできる
- [x] PlatformIOの`xiao_esp32c6`環境でビルドできる
- [ ] GitHub ActionsでS3/C3/C5/C6の4ジョブが成功し、各Artifactをダウンロードできる
- [x] GitHub PagesのWeb InstallerがHTTPSで表示される
- [x] GitHub Pagesのルートに製品ポータルが表示され、Web Installerへのリンクが機能する
- [x] Web Installerが`/ChainOSCPad/installer/`に表示される
- [x] OG／Twitterカードのメタデータが製品ポータルにだけ設定されている
- [x] Web InstallerがChainOSCシリーズ共通レイアウトとChainOSCPadの赤系配色で表示される
- [x] Web Installerの赤いキー配置アイコンがブラウザータブへ表示される
- [x] 32px favicon、Apple Touch Icon、OG画像がGitHub Pagesから取得できる
- [x] Web InstallerがXIAO ESP32S3を判定して書き込みできる
- [ ] Web InstallerがXIAO ESP32C3を判定して書き込みできる
- [x] Web InstallerがXIAO ESP32C5を判定して書き込みできる
- [x] C5用merged.binの`0x2000`にESPイメージヘッダー`0xE9`があり、Web Installer書き込み後に起動する
- [x] Web InstallerがXIAO ESP32C6を判定して書き込みできる
- [ ] `scripts/test_web_installer.ps1 -SkipBuild -CheckOnly`でS3/C3/C5/C6のmanifest振り分けと全ファイルを検証できる
- [x] Web Installerで書き込んだ各機種が起動し、AP Modeと既存設定画面を利用できる
- [x] XIAO ESP32C6へ書き込み、起動ログに製品名とバージョンが表示される
- [x] XIAO ESP32C6で以下のS3向け実機確認項目も同様に動作する
- [ ] XIAO ESP32C3をBOOTボタンを押しながらUSB接続し、Uploadできて正常に起動する
- [x] XIAO ESP32C5へUploadでき、正常に起動する
- [x] XIAO ESP32C5が5 GHz帯Wi-Fiへ接続できる
- [x] C5でEncoderを時計回りに1クリックずつ10回回し、診断ログを採取する
- [x] C5でEncoderを反時計回りに1クリックずつ10回回し、診断ログを採取する
- [x] C5でEncoderを通常速度と速い速度で各10クリック回し、欠落数と`max-gap`を比較する
- [x] C5の診断ログで通常時の`max-gap`が50 ms前後から十分短くなったことを確認する
- [x] C5の起動ログに`[Web] idle delay disabled for ESP32C5`と表示される
- [x] C5診断サマリーから約50 ms停止する処理がキーマトリクス走査だと特定できる
- [x] C5でROWをopen-drain固定した後、診断ログの`matrix`が約50 msから十分短くなる
- [x] C5でKey 1～12と複数同時押しが従来どおり動作する
- [x] C5を数分間動作させてもwatchdog resetやWi-Fi切断が発生しない

### XIAO ESP32C6 大容量設定

- [x] Key 1～12へ最大容量のプリセットを保存できる
- [x] 12 Key保存後もLittleFSに十分な空き容量がある（使用57,344／総容量1,441,792 bytes）
- [x] 再起動後にKey 1～12の設定がすべて復元される
- [x] 最大容量設定の状態でもWeb UIにKey 12とEncoderまで表示される
- [x] 最大容量設定のKey 1～12からOSCを送信できる
- [x] 最大容量の全体設定JSONをエクスポートし、再インポートできる

## 言語

- [x] 初回アクセス時にブラウザーの言語が日本語なら日本語、それ以外なら英語になる
- [x] `English`／`日本語`を切り替えてもURLが変わらない
- [x] 再起動後も選択した言語が保持される
- [x] ネットワーク、入力設定、追加したOSC行、確認・エラー表示が選択言語になる
- [x] 通常のルート画面に言語、システム、WiFi、JSON、OSC、Key 1～12、Encoderが順番どおり表示される
- [x] XIAO ESP32S3版のシステム欄に`XIAO ESP32S3`と表示される
- [x] XIAO ESP32C3版のシステム欄に`XIAO ESP32C3`と表示される
- [x] XIAO ESP32C5版のシステム欄に`XIAO ESP32C5`と表示される
- [x] XIAO ESP32C6版のシステム欄に`XIAO ESP32C6`と表示される
- [x] システム欄に製品名、ハードウェア、バージョン、IPアドレス、mDNS名が表示される
- [x] 横幅の広いブラウザーで通常画面の左右に適度な余白が表示される
- [x] 「ネットワーク」「入力設定」のページ切り替えナビゲーションが表示されない

## JSONバックアップとプリセット

- [x] 全体設定JSONをエクスポートでき、`format`が`ChainOSCPad-settings`である
- [x] 全体設定JSONにWi-FiのSSIDとパスワードが含まれない
- [x] 全体設定JSONでOSC送信先、UI言語、12 Key、Encoderを復元できる
- [x] 再起動後もインポートした設定が保持される
- [x] 各Keyの`…`メニューからKeyプリセットを入出力できる
- [x] Encoderの`…`メニューからEncoderプリセットを入出力できる
- [x] 各Keyの`…`メニューから、そのKeyだけを初期化できる
- [x] Encoderの`…`メニューから、Encoderだけを初期化できる
- [x] 個別初期化時に確認ダイアログが表示され、ほかのデバイス設定は変化しない
- [x] プリセットの`format`が`ChainOSC-device-preset`である
- [x] M5ChainOSC／ChainOSCminiの同種デバイスプリセットをインポートできる
- [x] 全体設定JSONをプリセット欄へ指定すると拒否される
- [x] 異なるデバイス種類、破損JSON、不正なAddress、9件以上のメッセージを拒否する
- [x] 16 KiBを超えるプリセットと64 KiBを超える全体設定を拒否する
- [x] 12 Keyを最大容量にした全体設定JSONをエクスポートし、再インポートできる

### Device Preset Import Error Registry v1

- [ ] 共通fixtureのKey向けinvalid JSONが、`expected-errors.json`どおりのError Codeで拒否される
- [ ] 共通fixtureのEncoder向けinvalid JSONが、`expected-errors.json`どおりのError Codeで拒否される
- [ ] Angle、ToF、Joystickなど対応済みの異種プリセットが`E_PRESET_DEVICE_TYPE_MISMATCH`で拒否される
- [ ] 未知の`deviceType`が`E_PRESET_DEVICE_TYPE_UNSUPPORTED`で拒否される
- [ ] 0バイト、16 KiB超過、JSON構文不正、format不正、schemaVersion不正を対応するError Codeで拒否する
- [ ] エラー表示を日本語と英語へ切り替えても、原因と修正方法が一致する
- [ ] 拒否後に画面上の設定、保存済み設定およびOSC送信内容が変化しない
- [ ] 再起動後もインポート前の設定が維持される
- [ ] 正常なKey／Encoderプリセットは従来どおりインポート、保存、OSC送信できる

## 初回セットアップ

- [x] `ChainOSCPad-Setup`が表示される
- [x] パスワード`12345678`で接続できる
- [x] `http://192.168.4.1/`にSSIDとパスワードだけのWi-Fi設定画面が表示される
- [x] キャプティブポータルに言語、システム、JSON、OSC、入力設定が表示されない
- [x] Wi-Fi設定を保存すると再起動する
- [x] 保存した2.4 GHz Wi-Fiへ接続する
- [x] ESP32C5のキャプティブポータルに2.4 GHz／5 GHz対応と表示される
- [x] ESP32S3／C3／C6のキャプティブポータルに2.4 GHz対応と表示される
- [x] シリアルへ接続IPとOSC送信先が表示される
- [x] `http://chainoscpad.local/`で設定画面が表示される

## 設定保持・復旧

- [x] USB電源を抜き差ししても設定が保持される
- [x] 保存済みWi-Fiを停止して起動すると、約15秒後に設定APが表示される
- [x] 通常画面の「Wi-Fi設定を削除」で再起動し、設定APが表示される
- [x] Wi-Fi設定を削除してもOSC送信先と入力設定が保持される
- [x] Wi-Fiパスワードが通常画面のHTMLへ表示されず、AP画面にも保存値が埋め込まれない

## OSC入力

- [x] Key 1～12のPress／Releaseを受信できる
- [x] 複数キーを押した状態を保持できる
- [x] Encoderが`0.00`～`0.95`を循環送信する
- [x] Encoder Pushが`1.0`／`0.0`を送信する
- [x] Web設定で変更したHost／Portへ送信される
- [x] 通常画面で不正なOSC IPアドレス／UDPポートが拒否される
- [x] OSC送信先設定に個別の保存ボタンが表示されない
- [x] 「すべての設定を保存」でOSC送信先、Key 1～12、Encoderが一緒に保存される
- [x] 全体保存してもURLが変わらず、OSC送信先が再起動なしで反映される
- [x] OSC送信先または入力設定が不正な場合、どの設定も変更されない

## 入力設定

- [x] `http://chainoscpad.local/`に統合設定画面が表示される
- [x] Key／Encoderカードの折りたたみ、種別バッジ、ID表示が正しい
- [x] 接続済みバッジが表示されない
- [x] 画面をスクロールしても「すべての設定を保存」が追従する
- [x] 設定変更後、保存ボタン左に「未保存の変更があります」と表示される
- [x] 全体保存後、Key 1～12とEncoderの変更がすべて保持される
- [x] 起動時のシリアルログにLittleFSの使用量と総容量が表示される
- [x] 保存ログに各Key／EncoderファイルのサイズとLittleFS使用量が表示される
- [x] 全体保存してもブラウザーのURLがルート（`/`）から変化しない
- [x] ボタン、リンク、セレクトへマウスを重ねるとポインターカーソルになる
- [x] デバイス名を変更して再起動後も保持される
- [x] Key 1～12の設定が表示される
- [x] 全Keyへ最大件数の長い設定を保存してもKey 12とEncoderまで表示される
- [x] `/`で始まらないOSC Addressが拒否される
- [x] Float／Intへ変換できない値が拒否される
- [x] KeyのAddress、Press／Release値を変更して保存できる
- [x] KeyをFloat型で送信できる
- [x] KeyをInt型で送信できる
- [x] KeyをString型で送信できる
- [x] Press／Release合計8メッセージまで追加でき、9件目が追加されない
- [x] Press／Release合計8件で両方の追加ボタンがグレーアウトし、削除すると再び有効になる
- [x] PressまたはReleaseを0件にして、その操作では送信されない
- [x] 上下矢印で変更した順番どおりに複数メッセージが送信される
- [x] KeyのSequenceがStartからStepずつ進み、Endを超えるとStartへ戻る
- [x] Sequence欄が左帯なしの薄い枠で囲まれ、OSCアドレスだけが1行全幅で表示される
- [x] SequenceのStart／End／Step／Typeが横一列のタイルとして表示され、狭い画面では折り返される
- [x] 通常画面でKey 1～12が横3×縦4、Encoderが独立した配置ガイドとして表示される
- [x] キー配置ガイド見出しの左端に「▼」があり、押すと配置図と現在位置表示の両方が折りたたまれ、もう一度押すと展開される
- [x] 画面内に表示されているKey／Encoderカードがガイド上で強調され、複数カード表示時は複数箇所が強調される
- [x] Key／Encoderカード内をクリックまたは入力欄へフォーカスすると、該当する1つだけがガイド上で強調される
- [x] カード操作による単独強調の後にスクロールすると、画面内に表示中のカードを強調する状態へ戻る
- [x] 「現在の表示位置」に画面上端へ最も近い表示中カードが表示される
- [x] ガイドのKey／Encoderを押すと対応する設定カードへスクロールする
- [x] 設定カードを縦にスクロールしても配置ガイドが画面内へ追従する
- [x] Key／Encoderプリセットと全体設定のインポートで、Sequenceの`Step = 0`が拒否される
- [x] Key／Encoderプリセットと全体設定のインポートで、`Start < End`かつ`Step < 0`が拒否される
- [x] Key／Encoderプリセットと全体設定のインポートで、`Start > End`かつ`Step > 0`が拒否される
- [x] Web UIからの全体保存でも、上記3種類の不正なSequenceが拒否される
- [x] 不正なSequenceのインポート／保存が拒否された後も、保存済み設定が変更されていない
- [x] 正しいSequence設定をインポート／保存でき、Start／End／Stepが自動変更されない
- [x] Wi-Fi切断中にKeyのSequenceを押してもOSCが送信されず、Sequence値も進まない
- [x] Wi-Fi再接続後にKeyのSequenceを押すと、切断前の次の値から送信を再開する
- [x] Wi-Fi切断中にEncoder ClickのSequenceを押してもOSCが送信されず、Sequence値も進まない
- [x] Wi-Fi再接続後にEncoder ClickのSequenceを押すと、切断前の次の値から送信を再開する
- [x] Key／Encoder ClickのSequenceをString型で送信できる
- [x] EncoderをAbsoluteモードで送信できる
- [x] 出力範囲を`-1`～`1`に設定し、EncoderをIncrementモードで正負方向へ送信できる
- [x] Encoderの出力範囲とFloat／Int／String型が反映される
- [x] Encoder PushのPress／Release設定が反映される
- [x] Encoder Clickの複数メッセージ、0件、並べ替え、Sequenceが動作する
- [x] 電源を抜き差ししても入力設定が保持される
- [x] 設定リセット後、入力設定が初期値へ戻る

## ポータル／Release自動化

- [x] 製品ポータルから日本語／英語のクイックスタートとユーザーガイドを開ける
- [x] 製品ポータルからWeb Installerを開ける
- [ ] `scripts/test_web_installer.ps1 -SkipBuild -CheckOnly`でS3/C3/C5/C6の統合binとmanifestを検証できる
- [x] `vX.Y.Z`タグと`APP_VERSION`／Installer manifestのバージョンが異なる場合、Releaseビルドが失敗する
- [ ] 正しい`vX.Y.Z`タグでS3/C3/C5/C6の統合binとSHA-256を添付したDraft Releaseが作成される
- [x] Draft Releaseを公開すると、同じバージョンのファームウェアを使用するGitHub Pagesが公開される
- [x] 公開済みReleaseがないバージョンでは、現在のGitHub Pagesが置き換えられない
