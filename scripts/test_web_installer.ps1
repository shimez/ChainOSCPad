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
  @{ Environment = 'xiao_esp32s3'; ChipFamily = 'ESP32-S3' },
  @{ Environment = 'xiao_esp32c6'; ChipFamily = 'ESP32-C6' }
)

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
  $output = Join-Path $site "firmware\$environment"
  New-Item -ItemType Directory -Path $output -Force | Out-Null
  foreach ($name in @('bootloader.bin', 'partitions.bin', 'firmware.bin')) {
    $source = Join-Path $build $name
    if (-not (Test-Path -LiteralPath $source)) {
      throw "Missing $source. Build $environment first or omit -SkipBuild."
    }
    Copy-Item -LiteralPath $source -Destination (Join-Path $output $name) -Force
  }

  $bootApp = Get-ChildItem (Join-Path $env:USERPROFILE '.platformio\packages') -Recurse -File -Filter boot_app0.bin |
    Where-Object { $_.FullName -match '[\\/]tools[\\/]partitions[\\/]boot_app0\.bin$' } |
    Select-Object -First 1
  if (-not $bootApp) {
    throw 'boot_app0.bin was not found in the PlatformIO packages.'
  }
  Copy-Item -LiteralPath $bootApp.FullName -Destination (Join-Path $output 'boot_app0.bin') -Force
}

$config = Get-Content (Join-Path $repo 'include\config.h') -Raw
$versionMatch = [regex]::Match($config, 'APP_VERSION\[\]\s*=\s*"([^"]+)"')
if (-not $versionMatch.Success) {
  throw 'APP_VERSION was not found in include/config.h.'
}
$version = $versionMatch.Groups[1].Value
$manifestPath = Join-Path $site 'manifest.json'
$manifestText = (Get-Content $manifestPath -Raw).Replace('__VERSION__', $version)
$utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
[IO.File]::WriteAllText($manifestPath, $manifestText, $utf8WithoutBom)
$manifest = $manifestText | ConvertFrom-Json

$expectedParts = @(
  @{ Name = 'bootloader.bin'; Offset = 0 },
  @{ Name = 'partitions.bin'; Offset = 32768 },
  @{ Name = 'boot_app0.bin'; Offset = 57344 },
  @{ Name = 'firmware.bin'; Offset = 65536 }
)

foreach ($target in $targets) {
  $build = @($manifest.builds | Where-Object { $_.chipFamily -eq $target.ChipFamily })
  if ($build.Count -ne 1) {
    throw "manifest.json must contain exactly one $($target.ChipFamily) build."
  }
  if ($build[0].parts.Count -ne $expectedParts.Count) {
    throw "Unexpected part count for $($target.ChipFamily)."
  }
  for ($index = 0; $index -lt $expectedParts.Count; $index++) {
    $part = $build[0].parts[$index]
    $expected = $expectedParts[$index]
    $expectedPath = "firmware/$($target.Environment)/$($expected.Name)"
    if ($part.path -ne $expectedPath -or [int]$part.offset -ne $expected.Offset) {
      throw "Unexpected $($target.ChipFamily) part: $($part.path) at $($part.offset)."
    }
    if (-not (Test-Path -LiteralPath (Join-Path $site $part.path))) {
      throw "Installer file is missing: $($part.path)"
    }
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
