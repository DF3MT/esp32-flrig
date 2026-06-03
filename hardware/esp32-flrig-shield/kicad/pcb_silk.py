#!/usr/bin/env python3
"""Silkscreen labels for esp32-flrig-shield (absolute mm, F.SilkS)."""
from __future__ import annotations

from layout_compact import BOARD, PLACEMENTS

_POS = {r: (x, y, rot) for r, x, y, rot in PLACEMENTS}


def _gr(
    text: str,
    x: float,
    y: float,
    uuid: str,
    size: float = 0.7,
    thickness: float = 0.1,
    justify: str = "center",
    angle: float = 0,
) -> str:
    rot = f" {angle}" if angle else ""
    return f"""  (gr_text "{text}" (at {x} {y}{rot}) (layer "F.SilkS")
    (effects (font (size {size} {size}) (thickness {thickness})) (justify {justify}))
    (uuid {uuid}))
"""


def board_gr_texts() -> str:
    x1, y1, x2, y2 = BOARD
    cx = (x1 + x2) / 2
    lines = [
        _gr("ESP32-FLRIG", cx, y1 + 3.5, "pcb-silk-title", 1.0, 0.15),
        _gr("Rev D  78 x 50 mm  2-layer", cx, y1 + 5.2, "pcb-silk-rev", 0.65, 0.1),
        _gr("FRONT  radios plug here", cx, y1 + 9.5, "pcb-silk-front", 0.55, 0.08),
        _gr("BACK  J3 ESP32-S3 host   J7 rotor harness", cx, y2 - 2.8, "pcb-silk-back", 0.55, 0.08),
        _gr("CYD 2x10", 3.5, 24.0, "pcb-silk-cyd", 0.65, 0.1, "left"),
        _gr("S3", 41.0, y2 - 5.5, "pcb-silk-s3", 0.65, 0.1),
        _gr("ROTOR", 58.0, y2 - 5.5, "pcb-silk-rotor-hint", 0.65, 0.1),
    ]
    j5x, j5y = _POS["J5"][0], _POS["J5"][1]
    j6x, j6y = _POS["J6"][0], _POS["J6"][1]
    j1x, j1y = _POS["J1"][0], _POS["J1"][1]
    lines += [
        _gr("J5", j5x, j5y + 5.5, "pcb-silk-j5ref", 0.75, 0.12),
        _gr("RADIO B", j5x, j5y + 7.0, "pcb-silk-j5role", 0.6, 0.1),
        _gr("USB-A host", j5x, j5y + 8.2, "pcb-silk-j5type", 0.5, 0.08),
        _gr("J6", j6x, j6y + 5.5, "pcb-silk-j6ref", 0.75, 0.12),
        _gr("RADIO C", j6x, j6y + 7.0, "pcb-silk-j6role", 0.6, 0.1),
        _gr("USB-A host", j6x, j6y + 8.2, "pcb-silk-j6type", 0.5, 0.08),
        _gr("J1", j1x, j1y + 5.5, "pcb-silk-j1ref", 0.75, 0.12),
        _gr("RADIO A", j1x, j1y + 7.0, "pcb-silk-j1role", 0.6, 0.1),
        _gr("RJ45 CAT+AF", j1x, j1y + 8.2, "pcb-silk-j1type", 0.5, 0.08),
    ]
    return "\n".join(lines) + "\n"


FOOTPRINT_LABELS: dict[str, dict] = {
    "U1": {"ref_at": (52.0, 21.5), "value": "ISO7741", "value_at": (52.0, 23.0)},
    "U2": {"ref_at": (14.0, 27.5), "value": "AMS1117-3.3", "value_at": (14.0, 29.0)},
    "U3": {"ref_at": (22.0, 31.5), "value": "PCM5102", "value_at": (22.0, 33.0)},
    "U4": {"ref_at": (36.0, 31.5), "value": "INMP441", "value_at": (36.0, 33.0)},
    "U5": {"ref_at": (34.0, 10.5), "value": "USB2422", "value_at": (34.0, 12.0)},
    "J1": {"ref_at": (62.0, 10.0), "value": "RADIO A", "value_at": (62.0, 11.5)},
    "J2": {"ref_at": (10.5, 25.5), "value": "CYD 2x10", "value_at": (10.5, 27.0)},
    "J3": {"ref_at": (28.0, 48.5), "value": "ESP32-S3 HOST", "value_at": (28.0, 50.0)},
    "J5": {"ref_at": (18.0, 10.0), "value": "RADIO B USB", "value_at": (18.0, 11.5)},
    "J6": {"ref_at": (44.0, 10.0), "value": "RADIO C USB", "value_at": (44.0, 11.5)},
    "J7": {"ref_at": (58.5, 40.0), "value": "ROTOR", "value_at": (58.5, 41.5)},
}

J2_PIN_LABELS = [
    (3.2, 28.0, "1 5V"),
    (3.2, 30.54, "2 3V3"),
    (3.2, 33.08, "3 GND"),
    (3.2, 38.16, "5 TX"),
    (3.2, 40.7, "6 RX"),
    (3.2, 43.24, "7 BCLK"),
    (3.2, 45.78, "8 LRCK"),
    (3.2, 48.32, "9 DOUT"),
    (3.2, 50.86, "10 DIN"),
]

J3_PIN_LABELS = [
    (31.5, 34.0, "1 5V"),
    (31.5, 36.54, "2 DM"),
    (31.5, 39.08, "3 DP"),
    (31.5, 41.62, "4 GND"),
    (31.5, 44.16, "5 ROT-"),
    (31.5, 46.7, "6 ROT+"),
    (31.5, 49.24, "7 OC-"),
    (31.5, 51.78, "8 OC+"),
]

J7_PIN_LABELS = [
    (57.0, 34.0, "1 ROT-"),
    (57.0, 36.54, "2 ROT+"),
    (57.0, 39.08, "3 OC-"),
    (57.0, 41.62, "4 OC+"),
    (57.0, 44.16, "5 GND"),
    (57.0, 46.7, "6 GND"),
]


def pin_hint_gr_texts() -> str:
    out: list[str] = []
    for n, (x, y, t) in enumerate(J2_PIN_LABELS + J3_PIN_LABELS + J7_PIN_LABELS, 1):
        out.append(_gr(t, x, y, f"pcb-silk-pin-{n}", 0.45, 0.08, "left"))
    return "\n".join(out) + "\n"


def all_gr_text_block() -> str:
    return board_gr_texts() + pin_hint_gr_texts()


def _sub_property(
    block: str,
    kind: str,
    text: str,
    x: float,
    y: float,
    rot: float,
    *,
    layer: str = "F.SilkS",
    hidden: bool = False,
) -> str:
    hide_s = " (hide yes)" if hidden else ""
    new_line = (
        f'    (property "{kind}" "{text}" (at {x} {y} {rot}) (layer "{layer}") '
        f'(effects (font (size 0.8 0.8) (thickness 0.12))){hide_s})'
    )
    lines = block.splitlines()
    out: list[str] = []
    replaced = False
    for line in lines:
        is_hidden = "(hide yes)" in line
        if (
            not replaced
            and f'(property "{kind}"' in line
            and is_hidden == hidden
        ):
            out.append(new_line)
            replaced = True
        else:
            out.append(line)
    return "\n".join(out)


def patch_footprint_labels(block: str, ref: str) -> str:
    if ref not in FOOTPRINT_LABELS:
        return block
    info = FOOTPRINT_LABELS[ref]
    rot = _POS[ref][2]
    rx, ry = info["ref_at"]
    block = _sub_property(block, "Reference", ref, rx, ry, rot)
    vx, vy = info["value_at"]
    block = _sub_property(block, "Value", info["value"], vx, vy, rot, layer="F.SilkS")
    if ref == "U1":
        block = _sub_property(block, "Reference", ref, 0, 0, 0, hidden=True)
    return block
