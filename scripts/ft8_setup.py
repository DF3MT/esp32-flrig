#!/usr/bin/env python3
"""
Einheitliches FT8/WSJT-X Setup für ESP32 CAT Panel (Linux & Windows).

  python3 scripts/ft8_setup.py --list-devices
  python3 scripts/ft8_setup.py --config scripts/ft8_config.example.json
  python3 scripts/ft8_setup.py 192.168.4.1 --start
  python3 scripts/ft8_setup.py --write-config ~/.config/esp32-flrig/ft8.json

Abhängigkeiten: pip install -r scripts/requirements-ft8.txt
"""
from __future__ import annotations

import argparse
import json
import os
import platform
import subprocess
import sys
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parent.parent
FT8_REMOTE = REPO / "scripts" / "ft8_remote.py"
EXAMPLE_CFG = REPO / "scripts" / "ft8_config.example.json"

RADIO_HINTS: dict[str, str] = {
    "XIEGU_G90": "CI-V 19200, CAT am Display-IO. Hamlib: Xiegu G90 / IC-7100.",
    "XIEGU_X6100": "CI-V 19200 A4. USB Audio am Funk parallel möglich.",
    "XIEGU_X6200": "CI-V 19200. WSJT-X Modell X6200.",
    "YAESU_FT991A": "CAT 38400. Audio: USB oder LINE; FT8 USB/DATA-USB.",
    "YAESU_FT910": "CAT 38400 USB.",
    "YAESU_FTDX10": "CAT 38400.",
    "YAESU_FTDX101D": "CAT 38400, CAT-1 für Steuerung.",
    "YAESU_FTDX101MP": "Wie FT-DX101D.",
    "YAESU_FT891": "CAT 38400 Mini-DIN.",
    "YAESU_FT897": "CAT oft 4800 — Menü prüfen.",
    "YAESU_FT857": "CAT 4800.",
    "IC-7300": "CI-V 19200, Addr 94h.",
}


def load_config(path: Path | None) -> dict[str, Any]:
    cfg_path = path or EXAMPLE_CFG
    if not cfg_path.is_file():
        raise SystemExit(f"Config nicht gefunden: {cfg_path}")
    with cfg_path.open(encoding="utf-8") as f:
        return json.load(f)


def ensure_deps() -> None:
    try:
        import numpy  # noqa: F401
        import sounddevice  # noqa: F401
    except ImportError:
        req = REPO / "scripts" / "requirements-ft8.txt"
        cmd = [sys.executable, "-m", "pip", "install", "-r", str(req)]
        print("Installiere:", " ".join(cmd))
        subprocess.check_call(cmd)


def list_audio_devices() -> None:
    import sounddevice as sd

    print(sd.query_devices())
    print("\nGeräte-Index für ft8_config.json: play_device / rec_device")


def resolve_vb_cable_indices() -> tuple[int | None, int | None]:
    import sounddevice as sd

    devices = sd.query_devices()
    play_idx = rec_idx = None
    for i, d in enumerate(devices):
        name = d["name"]
        if d["max_output_channels"] > 0 and "CABLE Input" in name and play_idx is None:
            play_idx = i
        if d["max_input_channels"] > 0 and "CABLE Output" in name and rec_idx is None:
            rec_idx = i
    return play_idx, rec_idx


def test_cat(host: str, port: int) -> bool:
    target = f"{host}:{port}"
    for cmd in (
        ["rigctl", "-m", "2", "-r", target, "f"],
        ["wsl", "rigctl", "-m", "2", "-r", target, "f"],
    ):
        try:
            out = subprocess.run(cmd, capture_output=True, text=True, timeout=5)
            if out.returncode == 0 and out.stdout.strip():
                print(f"CAT OK: {out.stdout.strip()} Hz")
                return True
        except (FileNotFoundError, subprocess.TimeoutExpired):
            continue
    print("CAT: keine Antwort (rigctl installieren oder Host prüfen)")
    return False


def print_wsjtx_guide(cfg: dict[str, Any], play_name: str, rec_name: str) -> None:
    host = cfg.get("esp_host", "192.168.4.1")
    port = cfg.get("rigctld_port", 4532)
    model = cfg.get("radio_model", "")
    hint = RADIO_HINTS.get(model, "")
    print(
        f"""
╔══════════════════════════════════════════════════════════════╗
║  WSJT-X / FT8 — ESP32 CAT Panel                              ║
╠══════════════════════════════════════════════════════════════╣
║  Radio → Settings → Rig                                      ║
║    Rig: Hamlib NET rigctl   Model: 2                         ║
║    Host: {host:<20} Port: {port:<5}              ║
║    PTT: CAT    Mode: USB                                     ║
╠══════════════════════════════════════════════════════════════╣
║  Audio (48 kHz)                                              ║
║    Input:  {rec_name[:44]:<44} ║
║    Output: {play_name[:44]:<44} ║
╠══════════════════════════════════════════════════════════════╣
║  Reihenfolge: 1) ft8_remote  2) WSJT-X starten               ║
╚══════════════════════════════════════════════════════════════╝
"""
    )
    if hint:
        print(f"Funkprofil {model}: {hint}\n")


def start_bridge(cfg: dict[str, Any]) -> None:
    host = cfg["esp_host"]
    args = [
        sys.executable,
        str(FT8_REMOTE),
        host,
        "--port-out",
        str(cfg.get("audio_port_out", 4533)),
        "--port-in",
        str(cfg.get("audio_port_in", 4534)),
        "--esp-rate",
        str(cfg.get("esp_sample_rate", 48000)),
        "--host-rate",
        str(cfg.get("host_sample_rate", 48000)),
        "--block-ms",
        str(cfg.get("block_ms", 20)),
    ]
    play = cfg.get("play_device")
    rec = cfg.get("rec_device")
    if play is not None:
        args.extend(["--play-device", str(play)])
    if rec is not None:
        args.extend(["--rec-device", str(rec)])
    print("Starte:", " ".join(args))
    if platform.system() == "Windows":
        subprocess.Popen(args, cwd=str(REPO), creationflags=subprocess.CREATE_NEW_CONSOLE)
    else:
        subprocess.Popen(args, cwd=str(REPO))


def write_user_config(dst: Path, cfg: dict[str, Any]) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    with dst.open("w", encoding="utf-8") as f:
        json.dump(cfg, f, indent=2)
    print(f"Config geschrieben: {dst}")


def main() -> None:
    p = argparse.ArgumentParser(description="ESP32 FLRIG FT8 Setup")
    p.add_argument("host", nargs="?", help="ESP IP (überschreibt config)")
    p.add_argument("--config", "-c", type=Path, help="JSON config")
    p.add_argument("--write-config", type=Path, metavar="PATH", help="Beispiel-Config speichern")
    p.add_argument("--list-devices", action="store_true")
    p.add_argument("--test-cat", action="store_true")
    p.add_argument("--start", action="store_true", help="ft8_remote.py starten")
    p.add_argument("--skip-deps", action="store_true")
    args = p.parse_args()

    if args.write_config:
        write_user_config(args.write_config, load_config(args.config))
        return

    if not args.skip_deps:
        ensure_deps()

    if args.list_devices:
        list_audio_devices()
        return

    cfg = load_config(args.config)
    if args.host:
        cfg["esp_host"] = args.host

    if cfg.get("play_device") is None or cfg.get("rec_device") is None:
        play, rec = resolve_vb_cable_indices()
        if cfg.get("play_device") is None and play is not None:
            cfg["play_device"] = play
        if cfg.get("rec_device") is None and rec is not None:
            cfg["rec_device"] = rec
        if play is not None or rec is not None:
            print(f"VB-Cable erkannt: play={play} rec={rec}")

    import sounddevice as sd

    play_name = rec_name = "?"
    try:
        if cfg.get("play_device") is not None:
            play_name = sd.query_devices(cfg["play_device"])["name"]
        if cfg.get("rec_device") is not None:
            rec_name = sd.query_devices(cfg["rec_device"])["name"]
    except Exception:
        pass

    print_wsjtx_guide(cfg, play_name, rec_name)

    if args.test_cat:
        test_cat(cfg["esp_host"], cfg.get("rigctld_port", 4532))

    if cfg.get("play_device") is None or cfg.get("rec_device") is None:
        print("Audio-Indizes fehlen. --list-devices und ft8_config.json anpassen.")
        if platform.system() == "Linux":
            print("Linux: scripts/ft8_linux_pa.sh für PulseAudio-Hilfe")
        elif platform.system() == "Windows":
            print("Windows: scripts/ft8_windows_setup.ps1 -InstallVbCable")
        if not args.start:
            sys.exit(1)

    if args.start:
        start_bridge(cfg)
    else:
        print("Bridge starten: python3 scripts/ft8_setup.py --config <cfg> --start")


if __name__ == "__main__":
    main()
