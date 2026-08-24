# Version 0.3.0 実機確認

## 初回セットアップ

- [ ] `ChainOSCPad-Setup`が表示される
- [ ] パスワード`12345678`で接続できる
- [ ] `http://192.168.4.1/`で設定画面が表示される
- [ ] 不正なOSC Host／Portが拒否される
- [ ] Wi-FiとOSC送信先を保存すると再起動する
- [ ] 保存した2.4 GHz Wi-Fiへ接続する
- [ ] シリアルへ接続IPとOSC送信先が表示される
- [ ] `http://chainoscpad.local/`で設定画面が表示される

## 設定保持・復旧

- [ ] USB電源を抜き差ししても設定が保持される
- [ ] 保存済みWi-Fiを停止して起動すると、約15秒後に設定APが表示される
- [ ] 設定リセット後、再起動して設定APが表示される
- [ ] Wi-Fiパスワードが設定画面のHTMLへ表示されない

## OSC入力

- [ ] Key 1～12のPress／Releaseを受信できる
- [ ] 複数キーを押した状態を保持できる
- [ ] Encoderが`0.00`～`0.95`を循環送信する
- [ ] Encoder Pushが`1.0`／`0.0`を送信する
- [ ] Web設定で変更したHost／Portへ送信される

## 入力設定

- [ ] `http://chainoscpad.local/inputs`が表示される
- [ ] Key／Encoderカードの折りたたみ、種別バッジ、ID表示が正しい
- [ ] 接続済みバッジが表示されない
- [ ] 画面をスクロールしても「すべての設定を保存」が追従する
- [ ] 設定変更後、保存ボタン左に「未保存の変更があります」と表示される
- [ ] 全体保存後、Key 1～12とEncoderの変更がすべて保持される
- [ ] 起動時のシリアルログにLittleFSの使用量と総容量が表示される
- [ ] 保存ログに各Key／EncoderファイルのサイズとLittleFS使用量が表示される
- [ ] 全体保存してもブラウザーのURLが`/inputs`から変化しない
- [ ] ボタン、リンク、セレクトへマウスを重ねるとポインターカーソルになる
- [ ] デバイス名を変更して再起動後も保持される
- [ ] Key 1～12の設定が表示される
- [ ] 全Keyへ最大件数の長い設定を保存してもKey 12とEncoderまで表示される
- [ ] `/`で始まらないOSC Addressが拒否される
- [ ] Float／Intへ変換できない値が拒否される
- [ ] KeyのAddress、Press／Release値を変更して保存できる
- [ ] KeyをFloat型で送信できる
- [ ] KeyをInt型で送信できる
- [ ] KeyをString型で送信できる
- [ ] Press／Release合計8メッセージまで追加でき、9件目が追加されない
- [ ] Press／Release合計8件で両方の追加ボタンがグレーアウトし、削除すると再び有効になる
- [ ] PressまたはReleaseを0件にして、その操作では送信されない
- [ ] 上下矢印で変更した順番どおりに複数メッセージが送信される
- [ ] KeyのSequenceがStartからStepずつ進み、Endを超えるとStartへ戻る
- [ ] Key／Encoder ClickのSequenceをString型で送信できる
- [ ] EncoderをAbsoluteモードで送信できる
- [ ] 出力範囲を`-1`～`1`に設定し、EncoderをIncrementモードで正負方向へ送信できる
- [ ] Encoderの出力範囲とFloat／Int／String型が反映される
- [ ] Encoder PushのPress／Release設定が反映される
- [ ] Encoder Clickの複数メッセージ、0件、並べ替え、Sequenceが動作する
- [ ] 電源を抜き差ししても入力設定が保持される
- [ ] 設定リセット後、入力設定が初期値へ戻る
