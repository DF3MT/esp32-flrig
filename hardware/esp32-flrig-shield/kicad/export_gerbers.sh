#!/usr/bin/env bash
# Gerber + Bohrung für JLCPCB exportieren
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$ROOT/../../.." && pwd)"
PCB="$ROOT/esp32-flrig-shield.kicad_pcb"
OUT="$ROOT/../fabrication/gerbers"
ZIP="$ROOT/../fabrication/esp32-flrig-shield-jlc.zip"

if command -v kicad-cli &>/dev/null; then
  mkdir -p "$OUT"
  kicad-cli fab export gerbers --output "$OUT" "$PCB"
  kicad-cli fab export drill --output "$OUT" "$PCB"
  rm -f "$ZIP"
  (cd "$OUT" && zip -j "$ZIP" ./*.gbr ./*.drl 2>/dev/null || zip -j "$ZIP" ./*)
  echo "OK (KiCad): $ZIP"
  exit 0
fi

# Fallback ohne KiCad: Python + gerbonara
PYTHON="${PYTHON:-$REPO/.venv/bin/python}"
if [[ ! -x "$PYTHON" ]]; then
  PYTHON=python3
fi
"$PYTHON" "$ROOT/generate_gerbers.py" --zip --preview
echo "OK (Python): $ZIP"
echo "Preview: $ROOT/../fabrication/esp32-flrig-shield-preview.jpg"
