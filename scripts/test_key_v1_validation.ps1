param(
  [string]$BaseUri = 'http://chainoscpad.local',
  [ValidateRange(0, 11)]
  [int]$KeyIndex = 0
)

$ErrorActionPreference = 'Stop'
$base = $BaseUri.TrimEnd('/')

function Copy-Preset($Preset) {
  return $Preset | ConvertTo-Json -Depth 20 -Compress | ConvertFrom-Json
}

function Invoke-Preset($Preset) {
  $body = $Preset | ConvertTo-Json -Depth 20 -Compress
  return Invoke-WebRequest -Uri "$base/import_device_preset?index=$KeyIndex" `
    -Method Post -ContentType 'application/json' -Body $body `
    -SkipHttpErrorCheck
}

function Assert-Result([string]$Name, $Preset, [int]$Status,
                       [string]$ErrorCode = '') {
  $response = Invoke-Preset $Preset
  $content = [string]$response.Content
  if ($response.StatusCode -ne $Status -or
      ($ErrorCode -and -not $content.StartsWith("${ErrorCode}:"))) {
    throw "FAIL $Name status=$($response.StatusCode) body=$content"
  }
  Write-Host "PASS $Name status=$Status $ErrorCode"
}

$export = Invoke-WebRequest -Uri "$base/export_device_preset?index=$KeyIndex"
$canonical = $export.Content | ConvertFrom-Json
$testFailure = $null
$restoreFailure = $null

try {
  Assert-Result 'deviceTypeName canonical' (Copy-Preset $canonical) 200

  $case = Copy-Preset $canonical
  $case | Add-Member -NotePropertyName unknownRoot -NotePropertyValue $true
  Assert-Result 'unknown root field' $case 400 'E_PRESET_DEVICE_SETTING_INVALID'

  $case = Copy-Preset $canonical
  $case.key | Add-Member -NotePropertyName unknownKey -NotePropertyValue $true
  Assert-Result 'unknown Key field' $case 400 'E_PRESET_DEVICE_SETTING_INVALID'

  $case = Copy-Preset $canonical
  $case.key.press = @([pscustomobject]@{address='/test/key'; value='1'; type=1;
                                      unknownMessage=$true})
  Assert-Result 'unknown OSC Message field' $case 400 'E_PRESET_DEVICE_SETTING_INVALID'

  $case = Copy-Preset $canonical
  $case.key.sequence | Add-Member -NotePropertyName unknownSequence `
    -NotePropertyValue $true
  Assert-Result 'unknown Sequence field' $case 400 'E_PRESET_DEVICE_SETTING_INVALID'

  $case = Copy-Preset $canonical
  $case.deviceTypeName = 'Encoder'
  Assert-Result 'deviceTypeName wrong literal' $case 400 'E_PRESET_DEVICE_SETTING_INVALID'

  $case = Copy-Preset $canonical
  $case.PSObject.Properties.Remove('deviceTypeName')
  Assert-Result 'deviceTypeName missing' $case 400 'E_PRESET_REQUIRED_FIELD_MISSING'

  $case = Copy-Preset $canonical
  $case.deviceTypeName = 3
  Assert-Result 'deviceTypeName wrong type' $case 400 'E_PRESET_FIELD_TYPE_INVALID'

  $case = Copy-Preset $canonical
  $case.deviceTypeName = 'Encoder'
  $case.key.PSObject.Properties.Remove('mode')
  Assert-Result 'required field precedes wrong deviceTypeName literal' $case 400 `
    'E_PRESET_REQUIRED_FIELD_MISSING'

  $case = Copy-Preset $canonical
  $case.key.sequence.address = '/valid/address'
  Assert-Result 'canonical OSC Address' $case 200

  $case = Copy-Preset $canonical
  $case.key.sequence.address = ' /valid/address'
  Assert-Result 'OSC Address leading whitespace' $case 400 'E_OSC_ADDRESS_INVALID'

  $case = Copy-Preset $canonical
  $case.key.sequence.address = '/valid/address '
  Assert-Result 'OSC Address trailing whitespace' $case 400 'E_OSC_ADDRESS_INVALID'
} catch {
  $testFailure = $_
} finally {
  Write-Host 'Restoring original Key preset...'
  try {
    Assert-Result 'restore original preset' $canonical 200
  } catch {
    $restoreFailure = $_
    Write-Error 'RESTORE FAILED. Check the Key settings on the device.' `
      -ErrorAction Continue
  }
}

if ($testFailure -and $restoreFailure) {
  throw "Validation test failed: $($testFailure.Exception.Message) Restore also failed: $($restoreFailure.Exception.Message) Check the Key settings on the device."
}
if ($restoreFailure) {
  throw "Restore failed: $($restoreFailure.Exception.Message) Check the Key settings on the device."
}
if ($testFailure) {
  throw $testFailure
}
Write-Host 'PASS ChainOSCPad Device Preset v1 Key validation regression suite'
