#!/usr/bin/env python3
"""
FT8 / WSJT-X Audio- und CAT-Bridge zum ESP32 CAT Panel über WiFi.

  Funk RX  → ESP (UDP :4533) → dieses Skript → WSJT-X Eingang (Radio Audio)
  WSJT-X Ausgang → dieses Skript → ESP (UDP :4534) → Funk TX (Line/Mic)

CAT: WSJT-X Hamlib „NET rigctl“ direkt auf ESP :4532 (kein Proxy nötig).

Abhängigkeiten:
  pip install sounddevice numpy

Beispiel:
  python3 scripts/ft8_remote.py 192.168.4.1 --list-devices
  python3 scripts/ft8_remote.py 192.168.4.1 --play-device 3 --rec-device 3
"""
from __future__ import annotations

import argparse
import socket
import struct
import sys
import threading
import time

import numpy as np

try:
    import sounddevice as sd
except ImportError:
    print("Install: pip install sounddevice numpy", file=sys.stderr)
    sys.exit(1)

AUDIO_MAGIC = 0x45535041
HDR_FMT = "<I I H"
HDR_SIZE = struct.calcsize(HDR_FMT)
HOST_RATE = 48000  # WSJT-X Standard
ESP_RATE_DEFAULT = 48000


def resample_pcm16(pcm: np.ndarray, from_rate: int, to_rate: int) -> np.ndarray:
    if from_rate == to_rate or len(pcm) == 0:
        return pcm
    n_out = max(1, int(round(len(pcm) * to_rate / from_rate)))
    x_in = np.arange(len(pcm), dtype=np.float64)
    x_out = np.linspace(0, len(pcm) - 1, n_out)
    return np.interp(x_out, x_in, pcm.astype(np.float64)).astype(np.int16)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="FT8 remote: WSJT-X audio <-> ESP32 WiFi bridge"
    )
    p.add_argument("host", nargs="?", help="ESP32 IP (z.B. 192.168.4.1)")
    p.add_argument("--port-out", type=int, default=4533, help="ESP → PC (Funk-Empfang)")
    p.add_argument("--port-in", type=int, default=4534, help="PC → ESP (FT8 TX Audio)")
    p.add_argument("--esp-rate", type=int, default=ESP_RATE_DEFAULT,
                   help="Sample-Rate ESP (config audio_sample_rate)")
    p.add_argument("--host-rate", type=int, default=HOST_RATE,
                   help="Sample-Rate WSJT-X (meist 48000)")
    p.add_argument("--play-device", type=int, default=None,
                   help="sounddevice ID: Ausgabe = WSJT-X *Eingang* (Funk-RX)")
    p.add_argument("--rec-device", type=int, default=None,
                   help="sounddevice ID: Aufnahme = WSJT-X *Ausgang* (Funk-TX)")
    p.add_argument("--list-devices", action="store_true", help="Audio-Geräte anzeigen")
    p.add_argument("--block-ms", type=int, default=20, help="Puffergröße in ms")
    return p.parse_args()


def list_devices() -> None:
    print(sd.query_devices())
    print("\nTypisch WSJT-X:")
    print("  Eingang (Radio):  --play-device  = Gerät, auf das der Funk-RX abgespielt wird")
    print("  Ausgang (Radio):  --rec-device   = Gerät, von dem WSJT-X TX kommt")


def main() -> None:
    args = parse_args()
    if args.list_devices:
        list_devices()
        return
    if not args.host:
        print("ESP32 IP fehlt. Beispiel: ft8_remote.py 192.168.4.1", file=sys.stderr)
        sys.exit(1)

    esp_addr = (args.host, args.port_in)
    block = max(128, args.host_rate * args.block_ms // 1000)
    seq = 0

    sock_tx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_rx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_rx.bind(("", args.port_out))
    sock_rx.settimeout(0.02)

    play_q: list[np.ndarray] = []
    lock = threading.Lock()

    def rx_loop() -> None:
        while True:
            try:
                data, _ = sock_rx.recvfrom(4096)
            except socket.timeout:
                continue
            if len(data) < HDR_SIZE:
                continue
            magic, _s, n = struct.unpack_from(HDR_FMT, data, 0)
            if magic != AUDIO_MAGIC or n == 0:
                continue
            esp_pcm = np.frombuffer(data, dtype="<i2", count=n, offset=HDR_SIZE)
            host_pcm = resample_pcm16(esp_pcm, args.esp_rate, args.host_rate)
            flt = host_pcm.astype(np.float32) / 32768.0
            with lock:
                if len(play_q) < 24:
                    play_q.append(flt)

    threading.Thread(target=rx_loop, daemon=True).start()

    def in_callback(indata, _frames, _time, status) -> None:
        nonlocal seq
        if status:
            print(status, file=sys.stderr)
        mono = indata[:, 0] if indata.ndim > 1 else indata
        pcm_host = (mono * 32767).clip(-32768, 32767).astype(np.int16)
        pcm_esp = resample_pcm16(pcm_host, args.host_rate, args.esp_rate)
        n = len(pcm_esp)
        hdr = struct.pack(HDR_FMT, AUDIO_MAGIC, seq, n)
        seq += 1
        sock_tx.sendto(hdr + pcm_esp.tobytes(), esp_addr)

    def out_callback(outdata, _frames, _time, status) -> None:
        if status:
            print(status, file=sys.stderr)
        with lock:
            if play_q:
                chunk = play_q.pop(0)
                nout = min(len(chunk), len(outdata))
                outdata[:nout, 0] = chunk[:nout]
                if nout < len(outdata):
                    outdata[nout:, 0] = 0
            else:
                outdata.fill(0)

    in_kw = {"device": args.rec_device} if args.rec_device is not None else {}
    out_kw = {"device": args.play_device} if args.play_device is not None else {}

    print(f"FT8 bridge → {args.host}")
    print(f"  Funk→PC  UDP :{args.port_out}  {args.esp_rate} Hz → host {args.host_rate} Hz")
    print(f"  PC→Funk  UDP :{args.port_in}")
    print(f"  play_device (WSJT-X IN) ={args.play_device}  rec_device (WSJT-X OUT) ={args.rec_device}")
    print(f"WSJT-X: Rig=Hamlib NET rigctl, Host={args.host}:4532, USB, PTT via CAT")
    print("Ctrl+C to stop")

    with sd.InputStream(
        samplerate=args.host_rate,
        channels=1,
        dtype="float32",
        blocksize=block,
        callback=in_callback,
        **in_kw,
    ), sd.OutputStream(
        samplerate=args.host_rate,
        channels=1,
        dtype="float32",
        blocksize=block,
        callback=out_callback,
        **out_kw,
    ):
        while True:
            time.sleep(0.2)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped")
