#!/usr/bin/env bash
# Serve the web flasher on http://127.0.0.1 (Web Serial works on localhost).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PORT="${FLASHER_PORT:-8765}"
FLASHER="$ROOT/flasher"

if [[ ! -f "$FLASHER/firmware/esp32-cyd/manifest.json" ]]; then
  echo "[flasher] No firmware assets found."
  echo "  Run: ./scripts/build_flasher_assets.sh"
  exit 1
fi

if ! ls "$FLASHER"/firmware/*/firmware.bin &>/dev/null; then
  echo "[flasher] firmware.bin missing – run ./scripts/build_flasher_assets.sh"
  exit 1
fi

echo "[flasher] http://127.0.0.1:$PORT/"
echo "[flasher] Press Ctrl+C to stop."
cd "$FLASHER"
exec python3 -m http.server "$PORT" --bind 127.0.0.1
