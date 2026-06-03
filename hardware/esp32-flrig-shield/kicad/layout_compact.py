#!/usr/bin/env python3
"""Compact 2-layer shield placement (mm). Front = low Y (plug access)."""
from __future__ import annotations

# Board outline (Edge.Cuts)
BOARD = (2.0, 2.0, 80.0, 52.0)  # x1, y1, x2, y2 → 78 × 50 mm
ZONE_INSET = 1.0

# (ref, x, y, rotation_deg)
PLACEMENTS: list[tuple[str, float, float, float]] = [
    # Front edge — host / radio connectors (cable exits forward)
    ("J5", 18.0, 6.0, 0),   # USB-A radio B
    ("J6", 44.0, 6.0, 0),   # USB-A radio C
    ("J1", 62.0, 6.0, 0),   # RJ45 radio A
    # Left — optional CYD stack header
    ("J2", 6.0, 28.0, 0),   # 2×10 CYD
    # Center — power, hub, isolation, audio
    ("U2", 14.0, 22.0, 0),  # 3.3 V LDO (near J2/J3 power)
    ("U5", 34.0, 16.0, 0),  # USB2422 hub (OTG + J5/J6)
    ("U1", 52.0, 16.0, 0),  # ISO7741 (ESP UART ↔ RJ45 CAT)
    ("U3", 22.0, 36.0, 0),  # PCM5102 I2S DAC
    ("U4", 36.0, 36.0, 0),  # INMP441 I2S mic
    # Back edge — ESP32-S3 host + rotor harness
    ("J3", 28.0, 34.0, 0),  # 1×8 S3 OTG + rotor (pins toward back)
    ("J7", 54.0, 34.0, 90),  # 1×6 rotor (parallel J3 pins 5–8)
]

# Pad anchor helpers (footprint at, pad offset) → absolute for routing
def j2_pin(n: int) -> tuple[float, float]:
    col, row = (n - 1) % 2, (n - 1) // 2
    x, y = PLACEMENTS[3][1], PLACEMENTS[3][2]
    return x + col * 2.54, y + row * 2.54


def j3_pin(n: int) -> tuple[float, float]:
    x, y = PLACEMENTS[9][1], PLACEMENTS[9][2]
    return x, y + (n - 1) * 2.54


def j5_usb() -> tuple[float, float]:
    return PLACEMENTS[0][1] - 1.25, PLACEMENTS[0][2]  # D+ approx


def j6_usb() -> tuple[float, float]:
    return PLACEMENTS[1][1] + 1.25, PLACEMENTS[1][2]


def j1_cat() -> tuple[float, float]:
    return PLACEMENTS[2][1], PLACEMENTS[2][2] + 2.54


def u5_at() -> tuple[float, float]:
    return PLACEMENTS[5][1], PLACEMENTS[5][2]


def u1_at() -> tuple[float, float]:
    return PLACEMENTS[6][1], PLACEMENTS[6][2]


def u3_at() -> tuple[float, float]:
    return PLACEMENTS[7][1], PLACEMENTS[7][2]


def u4_at() -> tuple[float, float]:
    return PLACEMENTS[8][1], PLACEMENTS[8][2]


def u2_at() -> tuple[float, float]:
    return PLACEMENTS[4][1], PLACEMENTS[4][2]
