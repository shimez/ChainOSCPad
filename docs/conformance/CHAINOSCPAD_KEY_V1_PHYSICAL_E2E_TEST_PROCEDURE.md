# ChainOSCPad Device Preset v1 Key Physical E2E Test Procedure

## 1. Purpose and scope

This procedure verifies the observable Device Preset v1 Key behavior of ChainOSCPad from the common specification through the firmware, physical Key input, and actual OSC output.

The reference hardware is XIAO ESP32S3. XIAO ESP32C3 and ESP32C6 builds are covered separately by build evidence; repeating the same MCU-independent Key semantics on every MCU is not required unless a board-specific failure is suspected.

This procedure does not test or change Encoder behavior, Device Preset v2 migration, WebUI design, persistence formats, firmware implementation, or the common specification.

## 2. Baseline and source of truth

Record the values used for the run.

| Item | Required baseline | Observed |
|---|---|---|
| ChainOSC specification commit | `b57494280b9f55c1b68aa12b390713dc5bb18005` | |
| ChainOSCPad implementation commit | commit containing `fc6102324071831c73686a8c16f59f7ca2245bb7` and `6791758a9e722ad0bc563a5c044ee9c7a512bc7c` | |
| ChainOSCPad branch | `main` | |
| Hardware | XIAO ESP32S3 | |
| PlatformIO environment | `xiao_esp32s3` | |
| Test date/time | — | |
| Tester | — | |

Normative inputs:

- ChainOSC `DEVICE_PRESET_FORMAT_V1.md`
- ChainOSC `DEVICE_PRESET_ERROR_REGISTRY_V1.md`
- ChainOSC `schemas/chainosc-device-preset-v1.schema.json`
- ChainOSC `test-data/device-presets/key-runtime-vectors.json`

## 3. Equipment and preparation

Required:

- ChainOSCPad XIAO ESP32S3 hardware with at least one working physical Key
- Development PC on the same IPv4 network
- PowerShell 7 or later
- Serial monitor at the baud rate configured by the firmware
- An OSC receiver that shows receive time/order, address, OSC type, and value
- The ChainOSCPad and ChainOSC repositories checked out at the recorded baselines

No repository-provided OSC receiver was found. Use an existing trusted OSC monitor/receiver; record its name and version below. Do not infer delivery success only from the firmware's send log.

```text
OSC receiver:
Version:
Receiver PC IPv4 address:
Receive port:
```

### 3.1 Build and flash

From the ChainOSCPad repository:

```powershell
platformio run -e xiao_esp32s3 -t upload
platformio device monitor -b 115200
```

If the configured monitor speed differs, use the value in `platformio.ini`.

### 3.2 Network and OSC destination

1. Connect ChainOSCPad to the test Wi-Fi network.
2. Obtain its IP address from the Serial line `[WiFi] IP=...` or the WebUI Wi-Fi card.
3. Open `http://chainoscpad.local/`; if mDNS is unavailable, use `http://<device-ip>/`.
4. Configure OSC destination address to the receiver PC IPv4 address and the recorded receive port.
5. Start the OSC receiver and verify one harmless known message can be received.
6. Record which physical Key corresponds to the zero-based WebUI/API index used below.

```text
ChainOSCPad IP:
BaseUri:
Physical Key:
KeyIndex (0..11):
```

### 3.3 Safety and restoration

Before changing the Key, export and retain its original Device Preset JSON:

```powershell
$BaseUri = 'http://chainoscpad.local'
$KeyIndex = 0
Invoke-WebRequest -Uri "$BaseUri/export_device_preset?index=$KeyIndex" -OutFile '.\key-original.json'
```

Use the actual `BaseUri` and `KeyIndex`. Confirm that `key-original.json` is valid JSON and identifies the intended Key. Do not begin destructive test configuration until this export succeeds.

At the end of every scenario, either restore the original preset or explicitly carry forward the documented test state needed by the next scenario. Final restoration is mandatory:

```powershell
Invoke-WebRequest -Uri "$BaseUri/import_device_preset?index=$KeyIndex" `
  -Method Post -ContentType 'application/json' `
  -InFile '.\key-original.json'
```

After restoration, export the Key again and compare its semantic JSON content with `key-original.json`. JSON property order and whitespace are not comparison criteria. Then press and release the Key once and confirm its normal behavior.

## 4. Evidence record format

For each scenario, complete the following fields.

```text
Test ID:
Purpose:
Covered requirement/vector:
Preconditions:
Observed result:
Result: PASS / FAIL
Evidence file or log:
Notes:
```

Capture OSC evidence with address, type, value, and order visible. Record Serial output when it is relevant to a detected failure or network transition.

## 5. Scenario HTTP-01 — Product-level HTTP validation

### Purpose

Run the reviewed 12-case product-level validation suite and prove that it restores the original Key preset on both its normal completion path and its guarded cleanup path.

### Preconditions

- Firmware and Wi-Fi setup from Section 3 are complete.
- The selected Key has a valid canonical Device Preset that can be exported.
- No other browser or script is modifying the selected Key.

### Steps

From the ChainOSCPad repository:

```powershell
.\scripts\test_key_v1_validation.ps1 `
  -BaseUri 'http://chainoscpad.local' `
  -KeyIndex 0
```

Replace the URI and index with the recorded values.

The suite covers canonical `deviceTypeName="Key"`, unknown fields at four structural levels, wrong/missing/wrong-type `deviceTypeName`, the composite priority case of wrong `deviceTypeName` plus missing `key.mode`, canonical OSC Address, and leading/trailing OSC Address whitespace.

### Expected result

- All 12 cases print `PASS` with their expected HTTP status and Error Registry code.
- The final line is `PASS ChainOSCPad Device Preset v1 Key validation regression suite`.
- `Restoring original Key preset...` and `PASS restore original preset status=200` are present.
- The PowerShell process exits successfully.
- A post-run export is semantically equal to the pre-run original preset.
- If any test fails, restoration is still attempted. A restoration failure must be visible and the process must exit non-zero.

### Failure handling

If the script reports `RESTORE FAILED`, stop all subsequent testing. Restore `key-original.json` manually through the WebUI or the command in Section 3.3, export again, and verify the setting before continuing.

```text
Observed 12-case result:
Restore message:
Post-run semantic comparison:
Result: PASS / FAIL
Evidence / notes:
```

## 6. Scenario KEY-01 — Press/Release order and duplicate preservation

### Covered vector

`KEY-PRESS-RELEASE-ORDER`

### Test preset

Configure Press/Release mode using one OSC address, for example `/chainosc/e2e/key/order`, with these String values in this exact order:

- Press: `first`, `duplicate`, `duplicate`, `last`
- Release: `release-first`, `release-last`

The total is six messages and is within the combined maximum of eight.

### Steps

1. Clear the OSC receiver log.
2. Press the selected physical Key once and hold it briefly.
3. Release it once.
4. Do not generate another Key transition until all six messages are visible.

### Expected result

The receiver shows exactly this order, with OSC type String:

```text
first
duplicate
duplicate
last
release-first
release-last
```

Both duplicate entries must be received. Press messages precede Release messages.

```text
Observed address/type/order:
Result: PASS / FAIL
Evidence / notes:
```

## 7. Scenario KEY-02 — Sequence start, ascending and descending endpoints

This combined scenario covers three vectors with two configurations.

### A. Start first and ascending endpoint inclusive

Covered vectors:

- `KEY-SEQUENCE-START-FIRST`
- `KEY-SEQUENCE-ASCENDING-END-INCLUSIVE`

Configure Sequence mode:

```text
Address: /chainosc/e2e/key/sequence
Type: Int
Start: 0
End: 3
Step: 1
```

Save the setting, then press the Key five times. The expected receiver values are:

```text
0, 1, 2, 3, 0
```

The first successful press must send Start, and End must be sent before wrapping.

### B. Descending endpoint inclusive

Configure:

```text
Address: /chainosc/e2e/key/sequence
Type: Int
Start: 3
End: 0
Step: -1
```

Save, clear the receiver, and press five times. Expected:

```text
3, 2, 1, 0, 3
```

```text
Ascending observed values:
Descending observed values:
Address/type:
Result: PASS / FAIL
Evidence / notes:
```

## 8. Scenario KEY-03 — Unreachable end without snapping, and Start equals End

### A. Unreachable End — no snap

Covered vector: `KEY-SEQUENCE-UNREACHABLE-END-NO-SNAP`

Configure:

```text
Address: /chainosc/e2e/key/sequence-float
Type: Float
Start: 0
End: 1
Step: 0.3
```

Press five times. Expected values, allowing only ordinary float display precision differences, are:

```text
0, 0.3, 0.6, 0.9, 0
```

The receiver must not show a synthesized `1.0` value.

### B. Start equals End

Covered vector: `KEY-SEQUENCE-START-EQUALS-END`

Configure:

```text
Address: /chainosc/e2e/key/sequence-equal
Type: Int
Start: 2
End: 2
Step: 1
```

Press three times. Expected:

```text
2, 2, 2
```

```text
Unreachable-end observed values:
Start-equals-End observed values:
Result: PASS / FAIL
Evidence / notes:
```

## 9. Scenario KEY-04 — Cold-start sequence position

### Covered vector

`KEY-SEQUENCE-COLD-START`

### Configuration

```text
Address: /chainosc/e2e/key/cold-start
Type: Int
Start: -2
End: 2
Step: 1
```

### Steps

1. Save the setting and clear the receiver.
2. Press at least three times and confirm the position has advanced beyond Start.
3. Power-cycle or reboot ChainOSCPad normally.
4. Wait for Wi-Fi reconnection; do not press the Key during boot.
5. Clear any unrelated receiver traffic.
6. Press the Key once.

### Expected result

- Reboot itself produces no Sequence OSC message.
- The first post-reboot press sends `-2` as Int.
- This demonstrates that Sequence `current` is reset to Start and is not persisted.

```text
Pre-reboot values:
First post-reboot value:
Result: PASS / FAIL
Evidence / notes:
```

## 10. Scenario KEY-05 — Invalid Import atomicity

### Covered vector

`KEY-INVALID-IMPORT-PRESERVES-STATE`

### Steps

1. Configure Sequence mode as Int with Start `0`, End `3`, Step `1`, and a unique address such as `/chainosc/e2e/key/atomicity`.
2. Save and press twice. Confirm values `0`, then `1`; the next runtime position is therefore `2`.
3. Export the current valid preset as `key-before-invalid.json`.
4. Create an invalid request from that exported JSON by setting `key.sequence.step` to `0`. Do not alter another field.
5. Import it with HTTP POST `application/json` or the WebUI.
6. Confirm HTTP 400 and `E_SEQUENCE_STEP_ZERO`.
7. Export again and compare it semantically with `key-before-invalid.json`.
8. Without rebooting or saving another setting, press the Key once.

### Expected result

- Invalid Import is rejected with `E_SEQUENCE_STEP_ZERO`.
- The persisted/exported preset remains unchanged.
- No OSC message is emitted by the Import attempt.
- The next physical press sends `2`, proving the active runtime position was not reset or mutated.

```text
HTTP status/Error Code:
Before/after semantic comparison:
Next physical value:
Result: PASS / FAIL
Evidence / notes:
```

After recording this scenario, restore the valid preset if another scenario depends on it.

## 11. Scenario KEY-06 — Detected send failure holds Sequence position

### Covered vector

`KEY-SEQUENCE-DETECTED-FAILURE-HOLDS-POSITION`

This vector is qualified: it applies only to a failure the product can detect. ChainOSCPad detects `WiFi.status() != WL_CONNECTED` before sending and returns failure without advancing Sequence position. UDP remote delivery failure is not detectable and must not be used as evidence.

### Configuration

```text
Address: /chainosc/e2e/key/detected-failure
Type: Int
Start: 0
End: 2
Step: 1
```

### Steps

1. Save the configuration while connected. Do not press the Key after saving; the test begins at Start `0`.
2. Confirm the exported preset contains the intended Sequence fields. Receiver connectivity was already proven in Section 3.2.
3. Make Wi-Fi unavailable without power-cycling or rebooting ChainOSCPad, for example by temporarily disabling the dedicated test access point.
4. Confirm Serial reports a disconnected Wi-Fi state. Press the Key once.
5. Confirm Serial contains `[OSC] skipped (WiFi disconnected)` for the configured address and that the receiver gets no message.
6. Restore the same access point and wait until the existing station connection recovers. Do not save settings or reboot.
7. Press the Key once.

### Expected result

- The disconnected press is a detected pre-send failure and produces no OSC packet.
- The next successful press after reconnection sends `0`, not `1`.
- Sequence position advances only after that successful send.

### Stability and safety note

Use a dedicated test access point whose temporary interruption does not affect other users or equipment. If the board reboots, enters setup/AP mode, or cannot restore the same station session without rebooting, mark this physical scenario `NOT EXECUTED` rather than treating it as a semantic failure. The common host-side runtime vector and inspection of `oscReady()`/`sendButton()` remain supporting evidence, but do not replace the physical result with a UDP receiver timeout.

```text
Disconnected Serial evidence:
Receiver result during failure:
First value after recovery:
Board rebooted or settings were saved during interval: YES / NO
Result: PASS / FAIL / NOT EXECUTED
Evidence / notes:
```

## 12. Final restoration and integrity check

1. Restore `key-original.json` using Section 3.3.
2. Export the selected Key again.
3. Compare the two JSON documents semantically.
4. Reboot ChainOSCPad.
5. Export once more and confirm the same original setting remains.
6. Perform one normal press/release and confirm the user's original behavior.

```text
Restore HTTP result:
Immediate semantic comparison:
Post-reboot semantic comparison:
Original runtime behavior:
Final restoration: PASS / FAIL
Evidence / notes:
```

Do not declare the overall Physical E2E PASS if final restoration fails.

## 13. Runtime vector coverage matrix

| Requirement / Vector | Coverage classification | Physical scenario | Required evidence | Result |
|---|---|---|---|---|
| Product-level HTTP validation, 12 cases | Physical | HTTP-01 | PowerShell output, restoration check | |
| `KEY-PRESS-RELEASE-ORDER` | Physical | KEY-01 | OSC address/type/ordered values including duplicate | |
| `KEY-SEQUENCE-START-FIRST` | Combined Physical | KEY-02A | First received value is Start | |
| `KEY-SEQUENCE-ASCENDING-END-INCLUSIVE` | Combined Physical | KEY-02A | `0,1,2,3,0` | |
| `KEY-SEQUENCE-DESCENDING-END-INCLUSIVE` | Combined Physical | KEY-02B | `3,2,1,0,3` | |
| `KEY-SEQUENCE-UNREACHABLE-END-NO-SNAP` | Combined Physical | KEY-03A | `0,0.3,0.6,0.9,0`; no `1.0` | |
| `KEY-SEQUENCE-START-EQUALS-END` | Combined Physical | KEY-03B | `2,2,2` | |
| `KEY-SEQUENCE-COLD-START` | Physical | KEY-04 | Pre-reboot progression and post-reboot Start | |
| `KEY-INVALID-IMPORT-PRESERVES-STATE` | Physical | KEY-05 | HTTP rejection, JSON equality, next runtime value | |
| `KEY-SEQUENCE-DETECTED-FAILURE-HOLDS-POSITION` | Physical, qualified | KEY-06 | Serial detected failure and same value after recovery | |

Supporting automated evidence is the common validator result for all nine runtime vectors. Record the command and result used for closure:

```powershell
node scripts/validate_device_preset_v2_fixtures.mjs
```

Run it from the ChainOSC repository and record its complete summary rather than editing fixtures during this procedure.

## 14. Final result

```text
HTTP validation: PASS / FAIL
KEY-01: PASS / FAIL
KEY-02: PASS / FAIL
KEY-03: PASS / FAIL
KEY-04: PASS / FAIL
KEY-05: PASS / FAIL
KEY-06: PASS / FAIL / NOT EXECUTED
Final restoration: PASS / FAIL

Overall Key v1 Physical E2E: PASS / FAIL
Open findings:
Evidence location:
```

If KEY-06 is `NOT EXECUTED`, record the environmental reason and retain both the automated vector PASS and implementation-inspection evidence. Conformance closure must state explicitly that the qualified physical failure case was not asserted; it must not silently report it as a physical PASS.
