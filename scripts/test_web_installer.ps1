[CmdletBinding()]
param(
  [switch]$SkipBuild,
  [switch]$CheckOnly,
  [ValidateRange(1024, 65535)]
  [int]$Port = 8765
)

$ErrorActionPreference = 'Stop'
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$pioRoot = [IO.Path]::GetFullPath((Join-Path $repo '.pio'))
$site = [IO.Path]::GetFullPath((Join-Path $pioRoot 'web-installer-test'))

if (-not $site.StartsWith($pioRoot, [StringComparison]::OrdinalIgnoreCase)) {
  throw 'The test site must stay inside the project .pio directory.'
}

$pioCommand = Get-Command pio -ErrorAction SilentlyContinue
if ($pioCommand) {
  $pio = $pioCommand.Source
} else {
  $pio = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\platformio.exe'
}
if (-not (Test-Path -LiteralPath $pio)) {
  throw 'PlatformIO was not found. Install PlatformIO or add pio to PATH.'
}

$python = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $python)) {
  throw 'The PlatformIO Python runtime was not found.'
}

if (Test-Path -LiteralPath $site) {
  Remove-Item -LiteralPath $site -Recurse -Force
}
New-Item -ItemType Directory -Path $site | Out-Null
Copy-Item -Path (Join-Path $repo 'docs\*') -Destination $site -Recurse -Force

$targets = @(
  @{ Environment = 'xiao_esp32s3'; ChipFamily = 'ESP32-S3'; Slug = 'XIAO-ESP32S3'; Chip = 'esp32s3'; FlashSize = '8MB' },
  @{ Environment = 'xiao_esp32c3'; ChipFamily = 'ESP32-C3'; Slug = 'XIAO-ESP32C3'; Chip = 'esp32c3'; FlashSize = '4MB' },
  @{ Environment = 'xiao_esp32c5'; ChipFamily = 'ESP32-C5'; Slug = 'XIAO-ESP32C5'; Chip = 'esp32c5'; FlashSize = '8MB' },
  @{ Environment = 'xiao_esp32c6'; ChipFamily = 'ESP32-C6'; Slug = 'XIAO-ESP32C6'; Chip = 'esp32c6'; FlashSize = '4MB' }
)

$config = Get-Content (Join-Path $repo 'include\config.h') -Raw
$versionMatch = [regex]::Match($config, 'APP_VERSION\[\]\s*=\s*"([^"]+)"')
if (-not $versionMatch.Success) {
  throw 'APP_VERSION was not found in include/config.h.'
}
$version = $versionMatch.Groups[1].Value
$firmwareDirectory = Join-Path $site 'installer\firmware'
New-Item -ItemType Directory -Path $firmwareDirectory -Force | Out-Null

foreach ($target in $targets) {
  $environment = $target.Environment
  if (-not $SkipBuild) {
    Write-Host "[Build] $environment"
    & $pio run --environment $environment
    if ($LASTEXITCODE -ne 0) {
      throw "PlatformIO build failed: $environment"
    }
  }

  $build = Join-Path $repo ".pio\build\$environment"
  foreach ($name in @('bootloader.bin', 'partitions.bin', 'firmware.bin')) {
    $source = Join-Path $build $name
    if (-not (Test-Path -LiteralPath $source)) {
      throw "Missing $source. Build $environment first or omit -SkipBuild."
    }
  }

  $bootApp = Get-ChildItem (Join-Path $env:USERPROFILE '.platformio\packages') -Recurse -File -Filter boot_app0.bin |
    Where-Object { $_.FullName -match '[\\/]tools[\\/]partitions[\\/]boot_app0\.bin$' } |
    Select-Object -First 1
  if (-not $bootApp) {
    throw 'boot_app0.bin was not found in the PlatformIO packages.'
  }
  $esptool = Get-ChildItem (Join-Path $env:USERPROFILE '.platformio\packages') -Recurse -File -Filter esptool.py |
    Where-Object { $_.FullName -match '[\\/]tool-esptoolpy[\\/]esptool\.py$' } |
    Select-Object -First 1
  if (-not $esptool) {
    throw 'esptool.py was not found in the PlatformIO packages.'
  }

  $firmwareName = "ChainOSCPad-$version-$($target.Slug)-merged.bin"
  $mergedFirmware = Join-Path $firmwareDirectory $firmwareName
  & $python $esptool.FullName --chip $target.Chip merge_bin -o $mergedFirmware `
    --flash_mode dio --flash_freq 80m --flash_size $target.FlashSize `
    0x0 (Join-Path $build 'bootloader.bin') `
    0x8000 (Join-Path $build 'partitions.bin') `
    0xe000 $bootApp.FullName `
    0x10000 (Join-Path $build 'firmware.bin')
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to create merged firmware: $environment"
  }
}

$manifestPath = Join-Path $site 'installer\manifest.json'
$manifestText = Get-Content $manifestPath -Raw
$manifest = $manifestText | ConvertFrom-Json

if ([string]$manifest.version -ne $version) {
  throw "manifest.json version $($manifest.version) does not match APP_VERSION $version."
}

foreach ($target in $targets) {
  $build = @($manifest.builds | Where-Object { $_.chipFamily -eq $target.ChipFamily })
  if ($build.Count -ne 1) {
    throw "manifest.json must contain exactly one $($target.ChipFamily) build."
  }
  if ($build[0].parts.Count -ne 1) {
    throw "Unexpected part count for $($target.ChipFamily)."
  }
  $part = $build[0].parts[0]
  $expectedPath = "firmware/ChainOSCPad-$version-$($target.Slug)-merged.bin"
  if ($part.path -ne $expectedPath -or [int]$part.offset -ne 0) {
    throw "Unexpected $($target.ChipFamily) part: $($part.path) at $($part.offset)."
  }
  if (-not (Test-Path -LiteralPath (Join-Path (Join-Path $site 'installer') $part.path))) {
    throw "Installer file is missing: $($part.path)"
  }
  Write-Host "[OK] $($target.ChipFamily) -> $($target.Environment)"
}

Write-Host "[OK] Web Installer manifest version $version"
Write-Host "[OK] Test site: $site"
if ($CheckOnly) {
  exit 0
}

$url = "http://127.0.0.1:$Port/installer/"
Write-Host "Open $url in desktop Chrome or Edge."
Write-Host 'Press Ctrl+C to stop the local server.'
& $python -m http.server $Port --bind 127.0.0.1 --directory $site
