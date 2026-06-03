#!/usr/bin/env bash
# WSL-aware flash helper for ESP32 CAT Remote Panel
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ENV="${1:-esp32-tdisplay}"
VENV_PIO="$ROOT/.venv/bin/pio"

cd "$ROOT"

# Build first
"$VENV_PIO" run -e "$ENV"

# WSL2: native USB (/dev/ttyUSB*) rarely available; Windows COM via ttyS* often fails.
# Prefer Windows-side esptool through PowerShell.
if [[ -n "${ESP_PORT:-}" ]]; then
    WIN_PORT="$ESP_PORT"
elif powershell.exe -NoProfile -Command "[System.IO.Ports.SerialPort]::getportnames()" 2>/dev/null | grep -q COM; then
    WIN_PORT="$(powershell.exe -NoProfile -Command "[System.IO.Ports.SerialPort]::getportnames()" | tr -d '\r' | head -1)"
else
    WIN_PORT=""
fi

if [[ -e /dev/ttyUSB0 || -e /dev/ttyACM0 ]]; then
    PORT="${ESP_PORT:-/dev/ttyUSB0}"
    echo "[flash] Native USB in WSL: $PORT"
    "$VENV_PIO" run -e "$ENV" -t upload --upload-port "$PORT"
elif [[ -n "$WIN_PORT" ]]; then
    echo "[flash] WSL detected – flashing via Windows $WIN_PORT"
    echo "[flash] (Close Arduino IDE / serial monitors using the port)"
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$ROOT/scripts/flash_tdisplay.ps1" -Port "$WIN_PORT" -EnvName "$ENV"
else
    echo "[flash] No serial port found."
    echo "  Option A: Install usbipd-win and attach USB to WSL"
    echo "  Option B: Connect board, then: ESP_PORT=COM5 ./scripts/flash.sh"
    exit 1
fi

# Upload LittleFS config (optional, via Windows if needed)
if [[ -f "$ROOT/data/config.json" ]]; then
    echo "[flash] To upload config.json: pio run -e $ENV -t uploadfs (requires USB in WSL or usbipd)"
fi
