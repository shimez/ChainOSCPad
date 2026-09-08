# ChainOSCPad P6 Physical E2E Test Procedure

Status: Draft for physical E2E  
Phase: P6 — Legacy WebUI / Explicit V2 Migration  
Date: 2026-09-08

## 1. Purpose

本手順は、実機ChainOSCPadと実ブラウザを使用し、次の境界が一連のproduct lifecycleとして成立することを確認する。

```text
Device Preset v1 non-lossless Import
  → Persisted Legacy
  → Legacy edit / ordinary Save / runtime / v1 Export
  → Explicit V2 Migration Candidate
  → Cancelまたはinvalid SaveではLegacy維持
  → validな明示Save成功時だけPersisted V2へpromotion
  → V2 runtime / v2 Export / reboot
```

Functional/state testとvisual/interaction testは別々に判定する。画面が正しく見えることだけで保存・runtimeをPASSにせず、機能が正しいことだけでFixed UIをPASSにしない。

## 2. Reference State

- Repository: `shimez/ChainOSCPad`
- Branch: `main`
- Inspected HEAD: `281f7b3e9fe4c84cd4f5dee30895f2b914c1011c`
- Test target: 上記HEADに、`src/network_manager.cpp`のP6 Legacy WebUI／explicit MigrationおよびFixed Encoder UIの未コミット差分を加えた状態
- Product source: `src/network_manager.cpp`, `src/input_settings.cpp`, `src/input_json.cpp`, `src/main.cpp`
- Common specification repository HEAD: `9652814bf1ffa867873aeeec36a831f84a6dc312`
- Guidelines: `<ChainOSC repository>/ChainOSC_WEBUI_DESIGN_GUIDELINES_V1.md`（Independent Review済みDraft）

## 3. Equipment and Preparation

### 3.1 Required equipment

- ChainOSCPad実機
- Encoderを操作できる組み立て済みhardware
- Windows PC
- ChainOSCPadと同一networkへ接続できるbrowser
- USB data cable
- PlatformIO
- OSC address、value、typeを表示できるOSC receiver

### 3.2 Fixture paths

Common repositoryを次とする。

```powershell
$ChainOscRepo = '<path-to-ChainOSC-repository>'
```

本手順で使用するfixture:

```text
test-data/device-presets-v2/migration/v1-amount-offset-input.json
test-data/device-presets-v2/migration/v1-amount-fractional-span-input.json
test-data/device-presets-v2/migration/v1-increment-string-input.json
test-data/device-presets-v2/migration/v1-amount-zero-based-input.json
```

いずれも公開v1としてvalidで、common classificationは`legacy-import`である。

### 3.3 Firmware and Serial Monitor

実機boardに対応するenvironmentを選ぶ。例としてXIAO ESP32S3では:

```powershell
Set-Location -LiteralPath '<path-to-ChainOSCPad-repository>'
pio run -e xiao_esp32s3 -t upload
pio device monitor -e xiao_esp32s3
```

他の実在environmentは`xiao_esp32c3`、`xiao_esp32c5`、`xiao_esp32c6`。monitor speedは`platformio.ini`により115200 bpsである。

### 3.4 WebUI and Import/Export

1. browserでChainOSCPad Settingsを開く。
2. Encoder card右上のDevice menu（`…`）を使用する。
3. Importは`プリセットをインポート（JSON）`を選び、対象fixtureを指定する。
4. Exportは`プリセットをエクスポート（JSON）`を選ぶ。
5. Export JSONの`schemaVersion`とEncoder fieldsをtext editorで確認する。

実装上のAPIは次のとおりだが、通常のE2EではWebUI操作を優先する。

- Import: `POST /import_device_preset?index=12`, `Content-Type: application/json`
- Export: `GET /export_device_preset?index=12`
- Migration start: `POST /encoder/start-v2-migration`
- Migration cancel: `POST /encoder/cancel-v2-migration`

### 3.5 Evidence record

各caseで次を記録する。

| Item | Record |
|---|---|
| Test date / tester | |
| Hardware / PlatformIO environment | |
| Firmware build identifier | |
| Browser / viewport | |
| Observed Result | |
| PASS / FAIL | |
| Evidence / Notes | screenshot、Export JSON、Serial、OSC output、reboot result |

## 4. Failure Handling

次の場合は後続testの前提が壊れるため、そのtest sequenceを停止する。

- Importが期待するmodelにならない
- ordinary SaveでV2へpromotionする
- Cancelまたはinvalid SaveでLegacyが失われる
- candidate表示だけでExportがv2になる
- valid Save成功後もV2へpromotionしない
- reboot後に直前のpersisted modelまたは設定を失う

FAIL時はcurrent badge/model、最後に成功した操作、browser URL、Serial log、HTTP表示、Export JSON、OSC output、reboot後stateを保存する。visual-only failureでstateが明らかに安全な場合は、機能testを別記録として継続してよい。

## 5. Part A — Functional / State / Persistence E2E

### P6-E2E-LEGACY-001 — v1 Offset Import、Legacy表示、runtime、Export

**Purpose:** non-lossless v1がPersisted Legacyとなり、P3/P4/P5/P6が接続されていることを確認する。

**Preconditions:** WebUIへ接続でき、OSC receiverが起動している。

**Input:** `v1-amount-offset-input.json`

主要fixture値:

- `sendIncrement=false`
- `wrapAround=false`
- `absoluteInputMin=-10`
- `absoluteInputMax=10`
- `incrementScale=0.05`
- `outMin=0`, `outMax=1`, `type=String`

**Steps:**

1. Encoder Device menuからfixtureをImportする。
2. Import成功表示後、画面をreloadする。
3. Legacy badgeと`旧形式のエンコーダー設定`を確認する。
4. Legacy固有fieldが上記fixture値と一致することを確認する。
5. Encoderを時計回り、反時計回りに操作し、`/migration/encoder`のOSC value/typeを記録する。
6. Encoder PresetをExportする。
7. Export JSONの`schemaVersion`とLegacy fieldsを確認する。
8. 実機を再起動し、再度WebUIとExportを確認する。

**Expected Result:**

- Import成功。Persisted modelはLegacy。
- Legacy Amount UIとして表示され、V2 `rangeSteps`へ置換されない。
- runtimeはLegacyのabsolute input offsetとStop semanticsを使用する。
- OSC typeはStringで、addressは`/migration/encoder`。
- promotion前のExportは`schemaVersion: 1`。
- reboot後もLegacy badge、field値、v1 Exportを維持する。

### P6-E2E-LEGACY-SAVE-001 — ordinary SaveはLegacyを維持

**Purpose:** Legacy editをexplicit migrationとして扱わないことを確認する。

**Preconditions:** P6-E2E-LEGACY-001完了後のLegacy Offset状態。

**Steps:**

1. Legacy fieldのうち、識別しやすい値を有効範囲内で変更する。例: `増分倍率=0.10`。
2. page-level `すべての設定を保存`を押す。
3. reloadし、変更値とLegacy badgeを確認する。
4. Exportし、`schemaVersion: 1`と変更値を確認する。
5. 再起動し、Legacy表示、変更値、runtimeを再確認する。

**Expected Result:** Save成功後もLegacy。V2 badgeやcandidate UIは表示されず、Exportはv1、reboot後もLegacy runtimeである。

### P6-E2E-MIG-START-001 — 明示的candidate作成はside-effect-free

**Purpose:** candidate作成とpromotionの境界を確認する。

**Preconditions:** Persisted Legacy Offset状態。開始直前のv1 Exportを保存しておく。

**Steps:**

1. Encoderを操作してruntimeが動作していることを確認する。
2. `v2設定へ移行する`を押す。
3. `v2 candidate` badgeとcandidate editorを確認する。
4. SaveせずEncoderを操作し、Legacy runtimeが継続することをOSCで確認する。
5. SaveせずDevice menuからExportする。
6. Exportが`schemaVersion: 1`で、開始前のLegacy設定と一致することを確認する。
7. browserをreloadする。
8. URLにmigration command/queryが残っていないことを確認する。

**Expected Result:**

- explicit actionでのみcandidateが表示される。
- candidate作成だけではstorage/model/runtime/Exportが変化しない。
- 現実装ではcandidateはdevice RAM上のvolatile stateとして保持されるため、同じboot中のreload後もcandidate表示を維持する。
- reloadはmigration commandを再実行せず、OSC送信やpromotionを発生させない。
- rebootした場合、未保存candidateは復元されずPersisted Legacyへ戻る。

### P6-E2E-MIG-CANCEL-001 — Candidate Cancel

**Purpose:** 未保存candidate編集がLegacyへ混入しないことを確認する。

**Preconditions:** Offset candidate表示中。

**Steps:**

1. candidateのOSCアドレス、Range Steps、出力範囲などを変更するがSaveしない。
2. `旧形式の設定へ戻る`を押す。
3. Legacy画面へ戻り、元のpersisted値を確認する。
4. Exportがv1であることを確認する。
5. Encoder runtimeがLegacyのままであることを確認する。
6. reloadし、candidateが再表示されないことを確認する。
7. 再起動後もLegacyであることを確認する。

**Expected Result:** Candidateは破棄され、編集値はLegacy、runtime、storage、Exportへ反映されない。Cancel自体でOSCを送信しない。

### P6-E2E-MIG-INVALID-001 — invalid Candidate Save

**Purpose:** validation failureではpromotionしないことを確認する。

**Preconditions:** `v2設定へ移行する`でOffset candidateを再作成する。

**Steps:**

1. confirmation checkboxを選択する。
2. `範囲ステップ数=0`にしてSaveする。
3. validation表示とSave失敗を記録する。
4. candidateが引き続き編集可能であることを確認する。
5. Device menuからExportし、`schemaVersion: 1`であることを確認する。
6. Encoderを操作し、Legacy runtimeであることを確認する。
7. 再起動する。
8. Legacy badge、元のLegacy field、v1 Exportを確認する。

**Expected Result:** Range Steps validationで拒否される。Persisted Legacyは変更されず、保存前candidateの間は編集を継続できる。reboot後はLegacyである。

### P6-E2E-MIG-SAVE-001 — valid SaveだけがV2へpromotion

**Purpose:** P6のpromotion boundaryを実証する。

**Preconditions:** 再起動後のPersisted Legacy Offset状態。

**Steps:**

1. `v2設定へ移行する`を押す。
2. candidateを次のvalid V2 Amount設定へ編集する。

   - OSC Address: `/migration/encoder`
   - Mode: 回転量
   - Output Min: `0`
   - Output Max: `1`
   - 最小値・最大値の先: ループする
   - Range Steps: `4`
   - Type: Float
   - 回転方向: 時計回りで大きくなる

3. semantic differenceを確認し、confirmation checkboxを選択する。
4. `すべての設定を保存`を押す。
5. Save成功後、reloadなしで通常V2 UIへ切り替わることを確認する。
6. V2 badge、candidate warningとCancel actionの消失を確認する。
7. Exportし、`schemaVersion: 2`と上記V2 fieldsを確認する。

**Expected Result:** product Save/readback成功時だけPersisted V2になる。成功後は通常V2 editorとなり、Exportはv2になる。

### P6-E2E-V2-RUNTIME-001 — promotion後のV2 runtime

**Purpose:** promotion後にV2 runtimeへ正しく接続されることを代表caseで確認する。

**Preconditions:** P6-E2E-MIG-SAVE-001のV2 Amount設定。OSC receiverを`/migration/encoder`で監視する。

**Steps:**

1. promotion直後からEncoderを時計回りに1 detentずつ5回操作する。
2. 各送信のaddress、value、OSC typeを記録する。
3. 必要なら反時計回りも操作し、方向を確認する。

**Expected Result:** 時計回りの代表系列はFloatで`0.25, 0.50, 0.75, 1.00, 0.00`。V2 Wrapは最大値を送信してから最小値へ戻る。

### P6-E2E-EXPORT-001 — promotion前後のmodel-aware Export

**Purpose:** P5 ExportとP6 lifecycleの接続を確認する。

**Evidence to compare:**

| State | Expected Export |
|---|---|
| Persisted Legacy | `schemaVersion: 1` |
| Candidate表示・編集中、未保存 | `schemaVersion: 1` |
| successful explicit V2 Save後 | `schemaVersion: 2` |

各JSONを別名で保存し、schemaVersionだけでなくEncoderのmodel固有fieldも記録する。

### P6-E2E-REBOOT-001 — promotion後のpersistence

**Purpose:** Persisted V2と設定値がreboot後も維持されることを確認する。

**Steps:**

1. P6-E2E-MIG-SAVE-001後に実機を再起動する。
2. V2 badgeと設定値を確認する。
3. Exportが`schemaVersion: 2`であることを確認する。
4. Encoderを操作し、P6-E2E-V2-RUNTIME-001と同じV2 semanticsを確認する。

**Expected Result:** reboot後もPersisted V2で、candidate UIはなく、V2設定・runtime・Exportを維持する。

## 6. Additional Candidate Coverage

各caseはImport後にcandidateを作成し、表示・candidate値・warningを記録した後、`旧形式の設定へ戻る`で終了する。これらをlossless migration成功とは記録しない。

### P6-E2E-CANDIDATE-FRACTIONAL-001

**Input:** `v1-amount-fractional-span-input.json`

**Expected candidate:**

- Amount
- copied: rotation address、output range/type、Wrap、Push
- `rangeSteps=0`のため、そのままではinvalid
- `1..65535`のRange Stepsをユーザーが入力する必要がある
- Legacy WrapとV2 Wrapの端点差warningを表示
- Cancel後は`absoluteInputMax=10.5`を含むLegacy v1が維持される

### P6-E2E-CANDIDATE-STRING-001

**Input:** `v1-increment-string-input.json`

**Expected candidate:**

- Direction
- Type: String
- counter-clockwise value: `-0.050`
- clockwise value: `0.050`
- Legacy IncrementをV2 Directionで表現するため、両方の値を確認するwarningを表示
- address、Pushはcopyされる
- Cancel後はLegacy Incrementとv1 Exportを維持する

### P6-E2E-CANDIDATE-WRAP-001

**Input:** `v1-amount-zero-based-input.json`

**Expected candidate:**

- Amount
- Range Steps: `20`
- Wrap: true
- Output Min/Max/Typeをcopy
- 次のFixed wordingを表示:

  `旧形式のループでは最大値を送信せず最小値に戻りますが、v2では最大値を送信してから最小値に戻ります（動作が変わります）`

- Cancel後はLegacy half-open Wrapとv1 Exportを維持する

## 7. Part B — Fixed Encoder WebUI Visual / Interaction E2E

### P6-UI-OVERALL-001 — Encoder全体構造

**Viewport:** 例 `1280 x 800`（800 pxより広いこと）

**Check:**

- Device header、model badge、Device menu
- identity `chainoscpad:encoder`、Import status
- デバイス名
- デバイス名とEncoder Rotationの間に明確な余白
- Encoder Rotationの直後にEncoder Push
- heading、field label、補足文の階層

CSSの22 px等は目視で定規測定せず、必要ならDevTools computed styleとGuidelinesの静的値を照合する。

### P6-UI-SECTIONS-001 — Rotation / Push treatment

**Check:**

- floating rounded cardではなくflat section
- outer borderとcorner radiusがない
- Rotation: orange left accent
- Push: green left accent
- left padding、section間隔、heading間隔に明らかな崩れがない
- PushがRotation直後にある

DevToolsで必要に応じて確認する静的値:

- top margin: 14 px
- left padding: 12 px
- accent width: 5 px
- corner radius: 0
- Rotation color: `#fd7e14`
- Push color: `#20c997`

### P6-UI-AMOUNT-001 — wide Amount layout

**Preconditions:** Persisted V2 AmountまたはAmount candidate。viewport width > 800 px。

**Expected layout:**

```text
OSCアドレス（full width）
モード（full width）
最小値 | 最大値 | 最小値・最大値の先
範囲ステップ数 | 型 | 回転方向
```

label、`🔄ループする`、`🛑停止する`、回転方向emojiを確認する。

### P6-UI-DIRECTION-001 — wide Direction layoutとfield対応

**Preconditions:** Persisted V2またはcandidateでDirectionを選択。viewport width > 800 px。

**Expected layout:**

```text
OSCアドレス（full width）
モード（full width）
↪️ 反時計回りの値 | ↩️ 時計回りの値 | 型
```

反時計回り欄へ`-0.125`、時計回り欄へ`0.375`を入力し、Direction→Amount→Directionと切り替える。値が入れ替わらず保持されることを確認する。保存する場合は実Encoderを両方向へ回し、表示labelと実際の送信値が一致することも確認する。

### P6-UI-LEGACY-001 — Legacy visual state

**Preconditions:** Legacy fixtureをImport済み。

**Check:**

- Legacy badge
- `旧形式のエンコーダー設定`がRotation heading直下
- status blockのspacing、border、background
- Legacy固有fieldがV2 fieldとして表示されない
- 通常SaveがLegacyを維持する説明
- `v2設定へ移行する`がstatus block最下部
- button上にhorizontal ruleがない

### P6-UI-CANDIDATE-001 — Candidate visual state

**Preconditions:** P6-E2E-CANDIDATE-WRAP-001またはOFFSET candidate。

**Check:**

- `v2 candidate` badge
- `v2移行候補を確認してください`がRotation heading直下
- semantic-difference warning
- confirmation checkboxが通常のfull-width text inputのように伸びていない
- `旧形式の設定へ戻る`がgrayの補助action
- primary Save、Migration開始、Cancelが視覚的に区別される
- Persisted V2と誤認しない
- offset caseでは次の文言:

  `旧形式の絶対値入力オフセットはv2回転量では表現されません`

### P6-UI-V2-001 — Persisted V2 visual state

**Preconditions:** P6-E2E-MIG-SAVE-001完了後。

**Check:**

- V2 badge
- normal V2 editor
- candidate warning、confirmation、`旧形式の設定へ戻る`が消えている
- Rotation／PushのFixed section treatmentが維持される

### P6-UI-SCROLL-001 — transition後の表示位置

次の3 transitionを個別に確認する。

1. Legacy → `v2設定へ移行する`
2. Candidate → `旧形式の設定へ戻る`
3. Candidate → successful Save → Persisted V2

**Expected Result:**

- transition後、Encoder Rotation headingと直下のLegacy/candidate blockがviewport内へ入る。
- page topへ戻ってEncoder設定が画面外にならない。
- browser URLに`encoder_v2_migration`等のcommand queryが残らない。
- transition直後に通常reloadしても、同じscroll要求を不必要に再実行しない。
- reloadはmigration commandやpromotionを再実行しない。

実装は`sessionStorage`の`focusEncoderMigration`を一度だけ消費する。DevToolsで確認する場合、transition完了後に当該keyが削除されていることを確認する。

### P6-UI-RESPONSIVE-001 — 800 px breakpoint

browser DevToolsのresponsive modeを使用する。

1. `1024 x 768`でAmountとDirectionを確認する。
2. `800 x 900`で同じ画面を確認する。
3. 必要なら`390 x 844`でも明らかなoverflowを確認する。

**Expected Result:**

| Width | Expected |
|---|---|
| > 800 px | V2／Legacy Rotationは3-column。Fixed field orderを維持 |
| <= 800 px | 1-column。全fieldを表示し、意味上の順序を維持 |

両条件でbadge、warning、Migration action、Cancel、label、emojiが欠落せず、horizontal overflowや重なりがないこと。

### P6-UI-SESSION-001 — reload / reboot / stale navigation

**Steps and Expected Result:**

1. Migration開始後にreload: 同一boot中はvolatile candidateを表示するが、commandを再実行せずpromotionしない。
2. Candidate表示中にreboot: candidateは消え、Persisted Legacyを表示する。
3. Cancel後にreload: Legacyのままでcandidateを再生成しない。
4. successful Save後にreload: Persisted V2のままでcandidateを再生成しない。
5. URLにmigration queryがないことを各段階で記録する。

## 8. Final Result Summary

| Gate | Result | Evidence |
|---|---|---|
| Legacy Import / UI / reboot | | |
| Legacy ordinary Save remains Legacy | | |
| Legacy runtime | | |
| Candidate creation is side-effect-free | | |
| Cancel preserves Legacy | | |
| Invalid Save preserves Legacy | | |
| Valid Save promotes to V2 | | |
| V2 runtime after promotion | | |
| v1→v2 Export boundary | | |
| V2 reboot persistence | | |
| Fixed Legacy UI | | |
| Fixed Candidate UI | | |
| Fixed V2 UI | | |
| Amount / Direction layout | | |
| Responsive behavior | | |
| Transition scroll / session behavior | | |

P6 Physical E2E全体をPASSとするには、Functional / State / PersistenceとFixed WebUI Visual / Interactionの両方がPASSでなければならない。
