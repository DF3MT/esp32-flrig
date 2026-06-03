#!/usr/bin/env bash
# PulseAudio: virtuelle Geräte für WSJT-X + ft8_remote.py (Linux)
set -euo pipefail

SINK_NAME="esp32-ft8-rx"
SOURCE_NAME="esp32-ft8-tx"

echo "[ft8] Erzeuge Null-Sink für Funk-Empfang (WSJT-X Eingang) …"
pactl load-module module-null-sink \
  sink_name="$SINK_NAME" \
  sink_properties=device.description="ESP32_FT8_Radio_RX" 2>/dev/null || true

echo "[ft8] Erzeuge Remapping-Source für WSJT-X Ausgang (TX zum Funk) …"
# Monitor des Sinks = was auf esp32-ft8-rx abgespielt wird (vom ft8_remote play)
# Für TX brauchen wir eine Aufnahme-Quelle: loopback von WSJT-X output
pactl load-module module-null-sink \
  sink_name="${SINK_NAME}-txloop" \
  sink_properties=device.description="ESP32_FT8_WSJTX_Out" 2>/dev/null || true

echo ""
echo "Geräte anzeigen:"
pactl list short sinks | grep -i esp32 || true
pactl list short sources | grep -i esp32 || true
echo ""
echo "1) ft8_remote.py mit --play-device = Sink-Monitor von $SINK_NAME"
echo "   pactl list sources | grep -i ${SINK_NAME}"
echo ""
echo "2) WSJT-X Eingang: Monitor of $SINK_NAME"
echo "3) WSJT-X Ausgang: ${SINK_NAME}-txloop (oder gleiches USB-Interface)"
echo ""
echo "Einfacher: ein USB-Soundgerät (z.B. reSpeaker) + ft8_remote.py --list-devices"
echo ""
echo "Komplett-Setup: ./scripts/ft8_setup.sh <ESP-IP> --pa --test-cat --start"
