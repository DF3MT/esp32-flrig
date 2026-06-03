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
FRAME = 320
SAMPLE_RATE = 16000


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="ESP32 transceiver audio over WiFi (UDP)")
    p.add_argument("host", help="ESP32 IP address")
    p.add_argument("--port-out", type=int, default=4533, help="listen: radio audio from ESP")
    p.add_argument("--port-in", type=int, default=4534, help="send: mic audio to ESP")
    p.add_argument("--rate", type=int, default=SAMPLE_RATE)
    return p.parse_args()


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
                data, _addr = sock_in.recvfrom(2048)
            except socket.timeout:
                continue
            if len(data) < HDR_SIZE:
                continue
            magic, _seq, n = struct.unpack_from(HDR_FMT, data, 0)
            if magic != AUDIO_MAGIC:
                continue
            pcm = np.frombuffer(data, dtype="<i2", count=n, offset=HDR_SIZE)
            with lock:
                if len(play_q) < 12:
                    play_q.append(pcm.astype(np.float32) / 32768.0)

    threading.Thread(target=rx_thread, daemon=True).start()

    def callback(indata, outdata, frames, _time, _status) -> None:
        nonlocal seq
        mono = (indata[:, 0] * 32767).astype(np.int16)
        hdr = struct.pack(HDR_FMT, AUDIO_MAGIC, seq, FRAME)
        seq += 1
        sock_out.sendto(hdr + mono.tobytes(), esp)

        with lock:
            if play_q:
                chunk = play_q.pop(0)
                n = min(len(chunk), len(outdata))
                outdata[:n, 0] = chunk[:n]
                outdata[n:, 0] = 0
            else:
                outdata.fill(0)

    print(f"Connected to {args.host}  in→:{args.port_in}  out←:{args.port_out} @ {args.rate} Hz")
    print("Ctrl+C to stop")
    with sd.Stream(
        samplerate=args.rate,
        channels=1,
        dtype="float32",
        blocksize=FRAME,
        callback=callback,
    ):
        while True:
            time.sleep(0.2)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped")
