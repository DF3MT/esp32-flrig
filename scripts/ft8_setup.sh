#!/usr/bin/env bash
# FT8 Setup (Linux) — Wrapper um ft8_setup.py + optional PulseAudio
set -euo pipefail
REPO="$(cd "$(dirname "$0")/.." && pwd)"
CFG="${FT8_CONFIG:-$REPO/scripts/ft8_config.example.json}"
PYTHON="${PYTHON:-python3}"

usage() {
  echo "Usage: $0 [ESP_IP] [--start] [--test-cat] [--pa] [--list]"
  echo "  --pa     PulseAudio Null-Sinks (ft8_linux_pa.sh)"
  echo "  --start  Audio-Bridge starten"
  exit 0
}

START=0
TEST_CAT=0
PA=0
LIST=0
HOST=""

for arg in "$@"; do
  case "$arg" in
    -h|--help) usage ;;
    --start) START=1 ;;
    --test-cat) TEST_CAT=1 ;;
    --pa) PA=1 ;;
    --list) LIST=1 ;;
    *)
      if [[ -z "$HOST" && "$arg" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        HOST="$arg"
      fi
      ;;
  esac
done

if [[ "$PA" -eq 1 ]]; then
  bash "$REPO/scripts/ft8_linux_pa.sh"
fi

ARGS=("$PYTHON" "$REPO/scripts/ft8_setup.py" "--config" "$CFG")
[[ -n "$HOST" ]] && ARGS+=("$HOST")
[[ "$LIST" -eq 1 ]] && ARGS+=("--list-devices")
[[ "$TEST_CAT" -eq 1 ]] && ARGS+=("--test-cat")
[[ "$START" -eq 1 ]] && ARGS+=("--start")

exec "${ARGS[@]}"
