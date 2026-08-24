# Version 0.4.2 実機確認

## 言語

- [x] 初回アクセス時にブラウザーの言語が日本語なら日本語、それ以外なら英語になる
- [x] `English`／`日本語`を切り替えてもURLが変わらない
- [x] 再起動後も選択した言語が保持される
- [x] ネットワーク、入力設定、追加したOSC行、確認・エラー表示が選択言語になる
- [x] ルート画面にWi-Fi／OSC、JSON、Key 1～12、Encoderがすべて表示される
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

## 初回セットアップ

- [x] `ChainOSCPad-Setup`が表示される
- [x] パスワード`12345678`で接続できる
- [x] `http://192.168.4.1/`で設定画面が表示される
- [x] 不正なOSC Host／Portが拒否される
- [x] Wi-FiとOSC送信先を保存すると再起動する
- [x] 保存した2.4 GHz Wi-Fiへ接続する
- [x] シリアルへ接続IPとOSC送信先が表示される
- [x] `http://chainoscpad.local/`で設定画面が表示される

## 設定保持・復旧

- [x] USB電源を抜き差ししても設定が保持される
- [x] 保存済みWi-Fiを停止して起動すると、約15秒後に設定APが表示される
- [x] 設定リセット後、再起動して設定APが表示される
- [x] Wi-Fiパスワードが設定画面のHTMLへ表示されない

## OSC入力

- [x] Key 1～12のPress／Releaseを受信できる
- [x] 複数キーを押した状態を保持できる
- [x] Encoderが`0.00`～`0.95`を循環送信する
- [x] Encoder Pushが`1.0`／`0.0`を送信する
- [x] Web設定で変更したHost／Portへ送信される

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
