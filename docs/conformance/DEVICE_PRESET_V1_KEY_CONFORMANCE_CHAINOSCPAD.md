# ChainOSCPad Device Preset v1 Key Conformance Record

## 1. Scope

This record closes ChainOSCPad conformance for the maintained Device Preset v1 Key contract. Key v1 is the current canonical Key format; it is not deprecated and is not a Legacy model. This record does not define or imply a Key v2 format or Key migration.

Encoder v2, Encoder migration, WebUI presentation, and unspecified future Sensor device formats are outside this closure scope.

## 2. Reviewed baseline

### Common specification

- Repository: `shimez/ChainOSC`
- Reviewed branch: `main`
- Reviewed commit: `b57494280b9f55c1b68aa12b390713dc5bb18005`
- Commit title: `Finalize Device Preset v1 Key specification`

The reviewed `main` HEAD was the recorded specification closure commit. No later normative Key changes existed at verification time.

Reviewed common assets:

- `DEVICE_PRESET_FORMAT_V1.md`
- `DEVICE_PRESET_ERROR_REGISTRY_V1.md`
- `schemas/chainosc-device-preset-v1.schema.json`
- Device Preset v1 Key fixtures
- `test-data/device-presets/key-runtime-vectors.json`

### ChainOSCPad implementation

- Repository: `shimez/ChainOSCPad`
- Reviewed branch: `main`
- Reviewed commit: `6791758a9e722ad0bc563a5c044ee9c7a512bc7c`
- Commit title: `Fix Key v1 validation priority and test cleanup`
- Prior implementation fix: `fc6102324071831c73686a8c16f59f7ca2245bb7`

At review time, local `main` was two commits ahead of `origin/main`; both commits named above were present and the product source had no later changes.

## 3. Verification methods

Results are kept distinct by evidence type:

1. **Automated common validation** — schema/fixture/migration/runtime-vector validator in the ChainOSC repository.
2. **Product source inspection** — ChainOSCPad validation, runtime, persistence, and detected-failure control flow.
3. **Product HTTP regression** — 12-case PowerShell test against physical firmware, including cleanup/restoration.
4. **Physical hardware E2E** — actual Key input, firmware runtime, OSC receiver output, reboot, Wi-Fi failure, and restoration on XIAO ESP32C5.
5. **Build verification** — independent PlatformIO builds; these do not claim physical execution on those targets.

## 4. Common validator

Command:

```powershell
node scripts/validate_device_preset_v2_fixtures.mjs
```

Observed current result:

```text
Encoder v2 valid:   11/11 PASS
Encoder v2 invalid: 20/20 PASS
Encoder migration:   8/8 PASS
Key v1 valid:       10/10 PASS
Key runtime:          9/9 PASS
failures=0
```

The Encoder counts are recorded only as regression evidence; they do not expand this record's Key scope.

## 5. Product validation closure

The original gaps are closed:

| Requirement | Verification | Result |
|---|---|---|
| Reject unknown root, Key, OSC Message, and Sequence fields | Source inspection and HTTP-01 | PASS |
| Require `deviceTypeName` | Source inspection and HTTP-01 | PASS |
| Require `deviceTypeName` JSON String type | Source inspection and HTTP-01 | PASS |
| Require the canonical literal `deviceTypeName == "Key"` | Source inspection and HTTP-01 | PASS |
| Reject OSC Address leading/trailing whitespace as `E_OSC_ADDRESS_INVALID` | Source inspection and HTTP-01 | PASS |
| Structural errors precede the wrong-literal semantic error | Composite HTTP-01 case | PASS |

The composite input with `deviceTypeName="Encoder"` and missing `key.mode` returned `E_PRESET_REQUIRED_FIELD_MISSING`. An otherwise valid Key preset with the wrong literal returned `E_PRESET_DEVICE_SETTING_INVALID`.

HTTP-01 result: **12/12 PASS**, plus successful original-preset restoration. Pre-test and post-restoration exports were identical.

## 6. Runtime-vector conformance

| Common runtime vector | Automated | Source inspection | Physical evidence | Result |
|---|---:|---:|---|---|
| `KEY-PRESS-RELEASE-ORDER` | PASS | PASS | KEY-01 PASS | PASS |
| `KEY-SEQUENCE-START-FIRST` | PASS | PASS | KEY-02 PASS | PASS |
| `KEY-SEQUENCE-ASCENDING-END-INCLUSIVE` | PASS | PASS | KEY-02 PASS | PASS |
| `KEY-SEQUENCE-DESCENDING-END-INCLUSIVE` | PASS | PASS | KEY-02 PASS | PASS |
| `KEY-SEQUENCE-UNREACHABLE-END-NO-SNAP` | PASS | PASS | KEY-03 PASS | PASS |
| `KEY-SEQUENCE-START-EQUALS-END` | PASS | PASS | KEY-03 PASS | PASS |
| `KEY-SEQUENCE-COLD-START` | PASS | PASS | KEY-04 PASS | PASS |
| `KEY-INVALID-IMPORT-PRESERVES-STATE` | PASS | PASS | KEY-05 PASS | PASS |
| `KEY-SEQUENCE-DETECTED-FAILURE-HOLDS-POSITION` | PASS | PASS | KEY-06 PASS | PASS |

Press and Release messages were emitted in configured order and duplicate messages were retained. Sequence emitted Start first, emitted reachable End before wrapping, did not synthesize an unreachable End, accepted `start == end` with a nonzero Step, and reset runtime-only current to Start after reboot.

## 7. Invalid Import atomicity

Physical state was advanced to runtime Sequence position `2`. Import of the same preset with only `key.sequence.step` changed to `0` was rejected with HTTP 400 and `E_SEQUENCE_STEP_ZERO`.

Verified after rejection:

- no OSC was emitted by the Import;
- persisted/exported settings were identical to the pre-Import settings;
- active settings were unchanged;
- the next physical Key press emitted `2`, proving runtime Sequence position was unchanged;
- invalid input was not rescued through fallback, correction, or default substitution.

Result: **PASS**.

## 8. Detected send failure qualification

The applicable product-detectable failure is the pre-send Wi-Fi disconnected condition. ChainOSCPad checks connection state before sending; when disconnected, the Sequence send returns failure and current position does not advance.

Physical Serial evidence included:

```text
[OSC] skipped (WiFi disconnected): /chainosc/e2e/key/detected-failure 0
```

No OSC packet was received during the detected failure. After recovery of the same AP, without reboot or Save, the receiver observed `0`, then `1`. This proves that the failed attempt did not advance position.

UDP remote delivery failure is not treated as detectable. This result makes no network acknowledgement, delivery, or retry guarantee.

Result: **PASS**.

## 9. Physical hardware E2E

Authoritative hardware: **Seeed Studio XIAO ESP32C5**.

| Scenario | Result |
|---|---|
| HTTP-01 — product validation and restoration | PASS |
| KEY-01 — Press/Release order and duplicates | PASS |
| KEY-02 — Start first and reachable endpoints | PASS |
| KEY-03 — unreachable End and Start equals End | PASS |
| KEY-04 — cold-start position | PASS |
| KEY-05 — invalid Import atomicity | PASS |
| KEY-06 — detected failure holds position | PASS |
| Final restoration and integrity | PASS |

Overall Key v1 Physical E2E: **PASS**. Open physical findings: **None**.

This result is not represented as physical execution on ESP32S3, ESP32C3, or ESP32C6.

## 10. Persistence and restoration

- HTTP-01 restored the original preset and produced an identical post-restore export.
- Final immediate re-export was binary-identical to the original JSON.
- Re-export after reboot was binary-identical to the original JSON.
- Original physical Key behavior was normal after restoration.

Result: **PASS**.

## 11. Build verification

Independent PlatformIO builds against the reviewed source:

| Environment | Result | Evidence classification |
|---|---|---|
| `xiao_esp32s3` | PASS | Build only |
| `xiao_esp32c3` | PASS | Build only |
| `xiao_esp32c5` | PASS | Build; also the separately recorded physical target |
| `xiao_esp32c6` | PASS | Build only |

Build success for S3/C3/C6 is not presented as physical E2E evidence.

## 12. Intentionally unspecified and product-specific behavior

This closure does not add requirements for areas intentionally left unspecified by the common Key v1 contract, including:

- exact Int conversion/rounding beyond the published field constraints;
- exact String formatting;
- successful Save/Import runtime-position reset behavior;
- browser reload, reconnect, and hot-reload behavior;
- exact floating-point comparison epsilon;
- some Press/Release partial-failure behavior;
- network delivery acknowledgement or retry.

Observed product behavior in these areas is not elevated into a common normative requirement by this record.

## 13. Limitations and N/A

- Physical E2E was executed only on XIAO ESP32C5.
- WebUI presentation and responsive behavior are out of scope.
- Encoder v1/v2, Legacy Encoder, and explicit Encoder migration are out of scope except for regression-boundary inspection/build evidence.
- Key v2 and Key migration are N/A because Key v1 remains the maintained canonical format.

## 14. Regression boundary

The reviewed product changes are limited to Key v1 validation and its regression script. No Encoder runtime, Encoder Push, Device Preset v2 Import/Export, Legacy Encoder compatibility/migration, persistence format, or WebUI behavior was changed by this closure work.

## 15. Evidence

- Physical procedure: `docs/conformance/CHAINOSCPAD_KEY_V1_PHYSICAL_E2E_TEST_PROCEDURE.md`
- Physical evidence: `docs/conformance/CHAINOSCPAD_KEY_V1_PHYSICAL_E2E_EVIDENCE.md`
- Product regression script: `scripts/test_key_v1_validation.ps1`
- Common runtime vectors: ChainOSC `test-data/device-presets/key-runtime-vectors.json`

## 16. Final verdict

```text
Overall verdict: PASS
ChainOSCPad Device Preset v1 Key Conformance: COMPLETE
Blocking findings: None
Non-blocking findings: None
```

The maintained ChainOSCPad Device Preset v1 Key contract is formally closed for the evidence and limitations recorded above.
