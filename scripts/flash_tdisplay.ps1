# Flash TTGO T-Display from Windows (WSL2: USB/COM is on Windows host)
param(
    [string]$Port = $env:ESP_PORT,
    [string]$EnvName = "esp32-tdisplay"
)

$ErrorActionPreference = "Stop"

if (-not $Port) {
    $ports = [System.IO.Ports.SerialPort]::getportnames()
    if ($ports.Count -eq 1) { $Port = $ports[0] }
    elseif ($ports -contains "COM5") { $Port = "COM5" }
    else { $Port = $ports | Select-Object -First 1 }
}

if (-not $Port) { throw "No COM port found. Connect TTGO T-Display via USB." }

$Repo = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Repo ".pio\build\$EnvName"

$files = @{
    "bootloader.bin" = 0x1000
    "partitions.bin" = 0x8000
    "firmware.bin"   = 0x10000
}
$bootApp0 = Join-Path $env:USERPROFILE ".platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"
if (-not (Test-Path $bootApp0)) {
    $bootApp0 = "\\wsl.localhost\Ubuntu\home\trunke_adm\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"
}

foreach ($f in $files.Keys) {
    $p = Join-Path $Build $f
    if (-not (Test-Path $p)) {
        $p = "\\wsl.localhost\Ubuntu\home\trunke_adm\git\flrig\.pio\build\$EnvName\$f"
    }
    if (-not (Test-Path $p)) { throw "Missing $f - run pio build first" }
}

Write-Host "Installing esptool (if needed)..."
python -m pip install -q esptool

Write-Host "Flashing $EnvName to $Port ..."
Write-Host "Tip: Hold BOOT (GPIO0) if upload fails."

$WslBase = "\\wsl.localhost\Ubuntu\home\trunke_adm\git\flrig\.pio\build\$EnvName"
$bl  = Join-Path $WslBase "bootloader.bin"
$pt  = Join-Path $WslBase "partitions.bin"
$fw  = Join-Path $WslBase "firmware.bin"
foreach ($p in @($bl, $pt, $fw, $bootApp0)) {
    if (-not (Test-Path $p)) { throw "Missing file: $p" }
}
$app = $bootApp0

python -m esptool --chip esp32 --port $Port --baud 921600 `
    --before default_reset --after hard_reset write_flash -z `
    --flash_mode dio --flash_freq 40m --flash_size detect `
    0x1000  $bl `
    0x8000  $pt `
    0xe000  $app `
    0x10000 $fw

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Done. Firmware flashed successfully."
