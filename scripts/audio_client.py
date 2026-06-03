#!/usr/bin/env python3
"""
Bidirektionaler UDP-Audio-Client für den ESP32 CAT Panel Audio-Bridge.

Empfang vom Funk (ESP → Client): UDP port_out (default 4533)
Mikro zum Funk (Client → ESP): UDP port_in  (default 4534)

Abhängigkeiten: pip install sounddevice numpy
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
HOST_RATE_DEFAULT = 48000
ESP_RATE_DEFAULT = 48000


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="ESP32 transceiver audio over WiFi (UDP)")
    p.add_argument("host", help="ESP32 IP address")
    p.add_argument("--port-out", type=int, default=4533, help="listen: radio audio from ESP")
    p.add_argument("--port-in", type=int, default=4534, help="send: mic audio to ESP")
    p.add_argument("--rate", type=int, default=HOST_RATE_DEFAULT,
                   help="Host sample rate (sounddevice)")
    p.add_argument("--esp-rate", type=int, default=ESP_RATE_DEFAULT,
                   help="ESP audio_sample_rate in config")
    return p.parse_args()


def resample_pcm16(pcm, from_rate: int, to_rate: int):
    if from_rate == to_rate or len(pcm) == 0:
        return pcm
    n_out = max(1, int(round(len(pcm) * to_rate / from_rate)))
    x_in = np.arange(len(pcm), dtype=np.float64)
    x_out = np.linspace(0, len(pcm) - 1, n_out)
    return np.interp(x_out, x_in, pcm.astype(np.float64)).astype(np.int16)


def main() -> None:
    args = parse_args()
    esp = (args.host, args.port_in)
    seq = 0

    sock_out = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_in = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_in.bind(("", args.port_out))
    sock_in.settimeout(0.05)

    play_q: list[np.ndarray] = []
    lock = threading.Lock()

    def rx_thread() -> None:
        while True:
            try:
                data, _addr = sock_in.recvfrom(4096)
            except socket.timeout:
                continue
            if len(data) < HDR_SIZE:
                continue
            magic, _seq, n = struct.unpack_from(HDR_FMT, data, 0)
            if magic != AUDIO_MAGIC or n == 0:
                continue
            esp_pcm = np.frombuffer(data, dtype="<i2", count=n, offset=HDR_SIZE)
            host_pcm = resample_pcm16(esp_pcm, args.esp_rate, args.rate)
            with lock:
                if len(play_q) < 24:
                    play_q.append(host_pcm.astype(np.float32) / 32768.0)

    threading.Thread(target=rx_thread, daemon=True).start()

    block = max(128, args.rate * 20 // 1000)

    def callback(indata, outdata, frames, _time, _status) -> None:
        nonlocal seq
        pcm_host = (indata[:, 0] * 32767).clip(-32768, 32767).astype(np.int16)
        pcm_esp = resample_pcm16(pcm_host, args.rate, args.esp_rate)
        n = len(pcm_esp)
        hdr = struct.pack(HDR_FMT, AUDIO_MAGIC, seq, n)
        seq += 1
        sock_out.sendto(hdr + pcm_esp.tobytes(), esp)

        with lock:
            if play_q:
                chunk = play_q.pop(0)
                n = min(len(chunk), len(outdata))
                outdata[:n, 0] = chunk[:n]
                outdata[n:, 0] = 0
            else:
                outdata.fill(0)

    print(f"Connected to {args.host}  in→:{args.port_in}  out←:{args.port_out}")
    print(f"  host {args.rate} Hz  esp {args.esp_rate} Hz  (FT8: scripts/ft8_remote.py)")
    print("Ctrl+C to stop")
    with sd.Stream(
        samplerate=args.rate,
        channels=1,
        dtype="float32",
        blocksize=block,
        callback=callback,
    ):
        while True:
            time.sleep(0.2)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped")
