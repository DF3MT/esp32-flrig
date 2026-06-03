#!/usr/bin/env bash
# Build firmware for all board envs and package binaries for the web flasher.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ -x "$ROOT/.venv/bin/pio" ]]; then
  PIO="$ROOT/.venv/bin/pio"
elif command -v pio >/dev/null 2>&1; then
  PIO="pio"
else
  echo "[flasher] PlatformIO not found. Run: python3 -m venv .venv && pip install platformio"
  exit 1
fi

export PLATFORMIO_CORE_DIR="${PLATFORMIO_CORE_DIR:-$ROOT/.pio-core}"

VERSION="$(git -C "$ROOT" describe --tags --always --dirty 2>/dev/null || echo dev)"
VERSION="${VERSION#v}"

find_boot_app0() {
  local candidates=(
    "$PLATFORMIO_CORE_DIR/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
    "$HOME/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
  )
  for p in "${candidates[@]}"; do
    if [[ -f "$p" ]]; then
      echo "$p"
      return 0
    fi
  done
  return 1
}

BOOT_APP0="$(find_boot_app0)" || {
  echo "[flasher] boot_app0.bin not found – run a PlatformIO build once."
  exit 1
}

ENVS=(esp32-cyd esp32-tdisplay esp32-generic)
NAMES=(
  "ESP32 CAT Panel (CYD)"
  "ESP32 CAT Panel (T-Display)"
  "ESP32 CAT Panel (Generic)"
)

echo "[flasher] Building firmware (version $VERSION) …"

for i in "${!ENVS[@]}"; do
  env="${ENVS[$i]}"
  name="${NAMES[$i]}"
  build_dir="$ROOT/.pio/build/$env"
  dest="$ROOT/flasher/firmware/$env"

  echo "[flasher]  → $env"
  "$PIO" run -e "$env"
  "$PIO" run -e "$env" -t buildfs

  for f in bootloader.bin partitions.bin firmware.bin; do
    if [[ ! -f "$build_dir/$f" ]]; then
      echo "[flasher] Missing $build_dir/$f"
      exit 1
    fi
  done

  mkdir -p "$dest"
  cp "$build_dir/bootloader.bin" "$build_dir/partitions.bin" "$build_dir/firmware.bin" "$dest/"
  cp "$BOOT_APP0" "$dest/boot_app0.bin"
  if [[ -f "$build_dir/littlefs.bin" ]]; then
    cp "$build_dir/littlefs.bin" "$dest/"
  fi

  python3 - "$dest" "$name" "$VERSION" <<'PY'
import json, os, sys
dest, name, version = sys.argv[1:4]
parts = [
    {"path": "bootloader.bin", "offset": 0x1000},
    {"path": "partitions.bin", "offset": 0x8000},
    {"path": "boot_app0.bin", "offset": 0xE000},
    {"path": "firmware.bin", "offset": 0x10000},
]
if os.path.isfile(os.path.join(dest, "littlefs.bin")):
    parts.append({"path": "littlefs.bin", "offset": 0x290000})
manifest = {
    "name": name,
    "version": version,
    "new_install_prompt_erase": True,
    "builds": [{"chipFamily": "ESP32", "parts": parts}],
}
out = os.path.join(dest, "manifest.json")
with open(out, "w", encoding="utf-8") as f:
    json.dump(manifest, f, indent=2)
    f.write("\n")
print(f"  wrote {out} ({len(parts)} parts)")
PY
done

echo "[flasher] Done. Open: ./scripts/serve_flasher.sh"
