#!/usr/bin/env python3
"""Re-apply compact placement from layout_compact.py to esp32-flrig-shield.kicad_pcb."""
from __future__ import annotations

import re
import sys
from pathlib import Path

from layout_compact import (
    BOARD,
    PLACEMENTS,
    ZONE_INSET,
    j1_cat,
    j2_pin,
    j3_pin,
    j5_usb,
    j6_usb,
    u1_at,
    u2_at,
    u3_at,
    u4_at,
    u5_at,
)
from pcb_silk import all_gr_text_block, patch_footprint_labels

PCB = Path(__file__).resolve().parent / "esp32-flrig-shield.kicad_pcb"
UUID_REF = {
    "u1": "U1", "u2": "U2", "u3": "U3", "u4": "U4", "u5": "U5",
    "j1": "J1", "j2": "J2", "j3": "J3", "j5": "J5", "j6": "J6", "j7": "J7",
}

SEGMENTS = [
    (3, 0.5, u2_at(), u5_at(), "seg-5v-hub"),
    (3, 0.4, u2_at(), j2_pin(1), "seg-5v-j2"),
    (3, 0.4, u5_at(), (18.0, 6.0), "seg-5v-j5"),
    (3, 0.4, u5_at(), (44.0, 6.0), "seg-5v-j6"),
    (14, 0.25, j3_pin(3), u5_at(), "seg-otg-dp"),
    (15, 0.25, j3_pin(2), u5_at(), "seg-otg-dm"),
    (16, 0.25, u5_at(), j5_usb(), "seg-usb-d1p"),
    (18, 0.25, u5_at(), j6_usb(), "seg-usb-d2p"),
    (4, 0.25, j2_pin(5), u1_at(), "seg-cat-tx-esp"),
    (5, 0.25, j2_pin(6), u1_at(), "seg-cat-rx-esp"),
    (6, 0.25, u1_at(), j1_cat(), "seg-cat-tx-rj"),
    (7, 0.25, u1_at(), (j1_cat()[0], j1_cat()[1] + 2.54), "seg-cat-rx-rj"),
    (8, 0.25, j2_pin(7), u3_at(), "seg-bclk"),
    (9, 0.25, j2_pin(8), u3_at(), "seg-lrck"),
    (10, 0.25, u3_at(), u4_at(), "seg-dout-din"),
    (11, 0.25, j2_pin(10), u4_at(), "seg-din"),
    (20, 0.25, (54.0, 34.0), j3_pin(5), "seg-rotor-1"),
    (21, 0.25, (54.0, 36.54), j3_pin(6), "seg-rotor-2"),
    (22, 0.25, (54.0, 39.08), j3_pin(7), "seg-rotor-3"),
    (23, 0.25, (54.0, 41.62), j3_pin(8), "seg-rotor-4"),
]


def main() -> int:
    x1, y1, x2, y2 = BOARD
    zx1, zy1 = x1 + ZONE_INSET, y1 + ZONE_INSET
    zx2, zy2 = x2 - ZONE_INSET, y2 - ZONE_INSET
    pos = {r: (x, y, rot) for r, x, y, rot in PLACEMENTS}

    text = PCB.read_text(encoding="utf-8")
    parts = re.split(r"(?=\(footprint )", text)
    header, fp_blocks = parts[0], parts[1:]

    def repl_at(block: str) -> str:
        m = re.search(r"\(uuid fp-([^)]+)\)", block)
        if not m:
            return block
        ref = UUID_REF.get(m.group(1).lower())
        if not ref or ref not in pos:
            return block
        x, y, r = pos[ref]
        block = re.sub(r"(\(at )[\d.]+ [\d.]+ [\d.]+", rf"\g<1>{x} {y} {r}", block, count=1)
        return patch_footprint_labels(block, ref)

    header = re.sub(
        r"\(gr_rect \(start [\d.]+ [\d.]+\) \(end [\d.]+ [\d.]+\)",
        f"(gr_rect (start {x1} {y1}) (end {x2} {y2})",
        header,
        count=1,
    )
    header = re.sub(r"\n  \(gr_text[\s\S]*\Z", "\n" + all_gr_text_block().rstrip(), header, count=1)
    header = re.sub(r"\n  \(zone[\s\S]*", "\n", header)

    def strip_after_footprints(block: str) -> str:
        m = re.search(r"\n  \((?:zone|segment)\b", block)
        if m:
            block = block[: m.start()]
        block = re.sub(r"\n\)\s*$", "", block.rstrip())
        return block.rstrip() + "\n"

    seg_txt = "\n".join(
        f'  (segment (start {a[0]} {a[1]}) (end {b[0]} {b[1]}) (width {w}) '
        f'(layer "F.Cu") (net {n}) (uuid {uid}))'
        for n, w, a, b, uid in SEGMENTS
    )
    zone = f"""  (zone (net 2) (net_name "GND_ESP") (layer "F.Cu") (uuid zone-gnd-f)
    (hatch edge 0.508)
    (connect_pads (clearance 0.3))
    (fill yes (thermal_gap 0.3) (thermal_bridge_width 0.4))
    (polygon
      (pts
        (xy {zx1} {zy1}) (xy {zx2} {zy1}) (xy {zx2} {zy2}) (xy {zx1} {zy2})
      )
    )
  )

"""
    fps = "".join(strip_after_footprints(repl_at(b)) for b in fp_blocks)
    out = header.rstrip() + "\n\n" + fps + zone + seg_txt + "\n)\n"
    PCB.write_text(out, encoding="utf-8")
    print(f"Updated {PCB} — {x2 - x1:.0f}×{y2 - y1:.0f} mm, {len(PLACEMENTS)} footprints")
    return 0


if __name__ == "__main__":
    sys.exit(main())
