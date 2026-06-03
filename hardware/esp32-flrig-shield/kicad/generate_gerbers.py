#!/usr/bin/env python3
"""
Export JLCPCB-ready Gerber + drill files from esp32-flrig-shield.kicad_pcb.

Requires: pip install kiutils gerbonara

Usage:
  python3 generate_gerbers.py
  python3 generate_gerbers.py --zip
"""
from __future__ import annotations

import argparse
import math
import re
import sys
import zipfile
from pathlib import Path

from gerbonara.apertures import CircleAperture, RectangleAperture, ExcellonTool
from gerbonara.cam import FileSettings
from gerbonara.excellon import ExcellonFile
from gerbonara.graphic_objects import Flash, Line, Region
from gerbonara.layers import LayerStack, NamingScheme
from gerbonara.rs274x import GerberFile
from gerbonara.utils import MM
from kiutils.board import Board

ROOT = Path(__file__).resolve().parent
PCB = ROOT / "esp32-flrig-shield.kicad_pcb"
OUT = ROOT.parent / "fabrication" / "gerbers"
BOARD = "esp32-flrig-shield"
MASK_GROW = 0.05  # mm solder mask expansion
EDGE_WIDTH = 0.15  # mm Edge.Cuts line width (KiCad gr_rect stroke)


def rotate_xy(x: float, y: float, deg: float) -> tuple[float, float]:
    if not deg:
        return x, y
    r = math.radians(deg)
    return x * math.cos(r) - y * math.sin(r), x * math.sin(r) + y * math.cos(r)


def drill_diameter(d) -> float:
    if d is None:
        return 1.0
    if hasattr(d, "diameter") and d.diameter is not None:
        return float(d.diameter)
    if hasattr(d, "X"):
        return float(max(d.X, d.Y if getattr(d, "Y", 0) else d.X))
    return float(d)


def pad_center(fp, pad) -> tuple[float, float]:
    px, py = pad.position.X, pad.position.Y
    if fp.position.angle:
        px, py = rotate_xy(px, py, fp.position.angle)
    return fp.position.X + px, fp.position.Y + py


def ref_name(fp) -> str:
    for prop in fp.properties or []:
        if isinstance(prop, dict):
            if prop.get("key") == "Reference":
                return prop.get("value", "?")
        elif getattr(prop, "key", None) == "Reference":
            return getattr(prop, "value", "?")
    # KiCad 7 property list as Property objects
    return getattr(fp, "reference", None) or "?"


def add_pad_copper(g: GerberFile, fp, pad, layer_filter: str) -> None:
    if layer_filter not in (pad.layers or []):
        if layer_filter == "F.Cu" and "F.Cu" not in (pad.layers or []):
            if "*.Cu" not in (pad.layers or []):
                return
        elif "*.Cu" in (pad.layers or []):
            pass
        else:
            return
    x, y = pad_center(fp, pad)
    if pad.type == "thru_hole" and pad.drill:
        dia = drill_diameter(pad.drill)
        g.objects.append(
            Flash(x, y, CircleAperture(dia, unit=MM), unit=MM)
        )
    elif pad.type == "smd" and pad.shape == "rect":
        w, h = pad.size.X, pad.size.Y
        g.objects.append(
            Flash(x, y, RectangleAperture(w, h, unit=MM), unit=MM)
        )


def add_edge_rectangle(edge: GerberFile, x1: float, y1: float, x2: float, y2: float) -> None:
    """Closed board outline as Edge.Cuts strokes (JLCPCB / gerbonara outline_svg_d)."""
    xa, xb = min(x1, x2), max(x1, x2)
    ya, yb = min(y1, y2), max(y1, y2)
    ap = CircleAperture(EDGE_WIDTH, unit=MM)
    edge.objects.append(Line(xa, ya, xb, ya, ap, unit=MM))
    edge.objects.append(Line(xb, ya, xb, yb, ap, unit=MM))
    edge.objects.append(Line(xb, yb, xa, yb, ap, unit=MM))
    edge.objects.append(Line(xa, yb, xa, ya, ap, unit=MM))


def add_pad_mask(g: GerberFile, fp, pad) -> None:
    layers = pad.layers or []
    if not any("Cu" in ly for ly in layers):
        return
    x, y = pad_center(fp, pad)
    if pad.type == "thru_hole" and pad.drill:
        dia = drill_diameter(pad.drill) + 2 * MASK_GROW
        g.objects.append(Flash(x, y, CircleAperture(dia, unit=MM), unit=MM))
    elif pad.type == "smd":
        g.objects.append(
            Flash(
                x,
                y,
                RectangleAperture(pad.size.X + 2 * MASK_GROW, pad.size.Y + 2 * MASK_GROW, unit=MM),
                unit=MM,
            )
        )


def build_stack() -> LayerStack:
    board = Board.from_file(str(PCB))
    raw = PCB.read_text()

    top_cu = GerberFile(generator_hints=["kicad", "esp32-flrig"])
    top_mask = GerberFile(generator_hints=["kicad", "esp32-flrig"])
    top_silk = GerberFile(generator_hints=["kicad", "esp32-flrig"])
    top_paste = GerberFile(generator_hints=["kicad", "esp32-flrig"])
    bottom_cu = GerberFile(generator_hints=["kicad", "esp32-flrig"])
    edge = GerberFile(generator_hints=["kicad", "esp32-flrig"])
    drill = ExcellonFile(generator_hints=["kicad", "esp32-flrig"])

    # Board outline — strokes only (filled Edge.Cuts breaks preview + outline_svg_d)
    edge_rect: tuple[float, float, float, float] | None = None
    for item in board.graphicItems:
        if getattr(item, "layer", None) == "Edge.Cuts" and hasattr(item, "start"):
            x1, y1 = item.start.X, item.start.Y
            x2, y2 = item.end.X, item.end.Y
            add_edge_rectangle(edge, x1, y1, x2, y2)
            edge_rect = (min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2))
    for m in re.finditer(
        r'\(gr_rect \(start ([\d.]+) ([\d.]+)\) \(end ([\d.]+) ([\d.]+)\)[^)]*\(layer "Edge.Cuts"',
        raw,
    ):
        x1, y1, x2, y2 = map(float, m.groups())
        if edge_rect is None:
            add_edge_rectangle(edge, x1, y1, x2, y2)
            edge_rect = (min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2))

    # Tracks
    for m in re.finditer(
        r'\(segment \(start ([\d.]+) ([\d.]+)\) \(end ([\d.]+) ([\d.]+)\) \(width ([\d.]+)\) \(layer "([^"]+)"',
        raw,
    ):
        x1, y1, x2, y2, w, layer = m.groups()
        x1, y1, x2, y2, w = map(float, (x1, y1, x2, y2, w))
        line = Line(x1, y1, x2, y2, CircleAperture(w, unit=MM), unit=MM)
        if layer == "F.Cu":
            top_cu.objects.append(line)
        elif layer == "B.Cu":
            bottom_cu.objects.append(line)

    # Copper pour (zone) — simplified rectangle from polygon
    for zone in board.zones:
        if "F.Cu" not in (zone.layers or []):
            continue
        if zone.polygons and zone.polygons[0].coordinates:
            pts = [(p.X, p.Y) for p in zone.polygons[0].coordinates]
            if len(pts) >= 3:
                top_cu.objects.append(Region(pts, unit=MM))

    # Footprints
    for fp in board.footprints:
        ref = ref_name(fp)
        for pad in fp.pads:
            add_pad_copper(top_cu, fp, pad, "F.Cu")
            add_pad_mask(top_mask, fp, pad)
            if pad.type == "smd" and "F.Paste" in (pad.layers or []):
                x, y = pad_center(fp, pad)
                top_paste.objects.append(
                    Flash(x, y, RectangleAperture(pad.size.X, pad.size.Y, unit=MM), unit=MM)
                )
            if pad.type == "thru_hole" and pad.drill:
                x, y = pad_center(fp, pad)
                dia = drill_diameter(pad.drill)
                drill.objects.append(
                    Flash(x, y, ExcellonTool(dia, plated=True, unit=MM), unit=MM)
                )
        # Silk reference
        top_silk.objects.append(
            Flash(
                fp.position.X,
                fp.position.Y,
                CircleAperture(0.15, unit=MM),
                unit=MM,
            )
        )

    # Silkscreen text from PCB
    for m in re.finditer(
        r'\(gr_text "([^"]*)"[^)]*\(at ([\d.]+) ([\d.]+)[^)]*\(layer "F.SilkS"',
        raw,
    ):
        tx, ty = float(m.group(2)), float(m.group(3))
        top_silk.objects.append(Flash(tx, ty, CircleAperture(0.2, unit=MM), unit=MM))

    stack = LayerStack(
        graphic_layers={
            ("top", "copper"): top_cu,
            ("top", "mask"): top_mask,
            ("top", "silk"): top_silk,
            ("top", "paste"): top_paste,
            ("bottom", "copper"): bottom_cu,
            ("mechanical", "outline"): edge,
        },
        drill_pth=drill,
        drill_npth=ExcellonFile(),
        board_name=BOARD,
        generator="kicad",
    )
    return stack


def validate_stack(stack: LayerStack) -> list[str]:
    """Sanity checks before JLCPCB upload."""
    issues: list[str] = []
    layers = stack.graphic_layers
    cu = len(layers.get(("top", "copper"), GerberFile()).objects)
    mask = len(layers.get(("top", "mask"), GerberFile()).objects)
    silk = len(layers.get(("top", "silk"), GerberFile()).objects)
    paste = len(layers.get(("top", "paste"), GerberFile()).objects)
    outline = len(layers.get(("mechanical", "outline"), GerberFile()).objects)
    drills = len(stack.drill_pth.objects) if stack.drill_pth else 0

    if cu < 10:
        issues.append(f"top copper has only {cu} objects (expected pours + tracks + pads)")
    if mask < 10:
        issues.append(f"top mask has only {mask} openings (expected all pads)")
    if outline < 4:
        issues.append(f"outline has {outline} objects (expected 4 Edge.Cuts lines)")
    if drills < 10:
        issues.append(f"PTH drill has only {drills} hits")
    if not stack.outline_svg_d(unit=MM):
        issues.append("outline_svg_d empty — Edge.Cuts must be lines/arcs, not filled regions")

    bb = stack.board_bounds(MM, default=None)
    if bb:
        w = bb[1][0] - bb[0][0]
        h = bb[1][1] - bb[0][1]
        if not (70 <= w <= 85 and 45 <= h <= 55):
            issues.append(f"board size {w:.1f}×{h:.1f} mm unexpected (target ~78×50 compact)")
    return issues


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip", action="store_true", help="Create JLCPCB upload ZIP")
    ap.add_argument("--preview", action="store_true", help="Render JPG/SVG preview (render_pcb_preview.py)")
    ap.add_argument("--validate", action="store_true", help="Run Gerber sanity checks and exit non-zero on failure")
    ap.add_argument("-o", "--output", type=Path, default=OUT)
    args = ap.parse_args()

    args.output.mkdir(parents=True, exist_ok=True)
    stack = build_stack()
    issues = validate_stack(stack)
    if issues:
        print("Validation warnings:")
        for msg in issues:
            print(f"  - {msg}")
    else:
        print("Gerber stack validation: OK")
    if args.validate and issues:
        sys.exit(1)

    settings = FileSettings.defaults()
    stack.save_to_directory(
        args.output,
        naming_scheme=NamingScheme.kicad,
        board_name=BOARD,
        gerber_settings=settings,
        excellon_settings=settings,
        overwrite_existing=True,
    )

    # JLCPCB likes a simple job note
    (args.output / "README_JLCPCB.txt").write_text(
        f"""JLCPCB Gerber upload — {BOARD}
================================
Upload ZIP: {BOARD}-jlc.zip (parent fabrication folder)

Layers included (KiCad naming):
  {BOARD}-F_Cu.gbr      Top copper
  {BOARD}-F_Mask.gbr    Top solder mask
  {BOARD}-F_SilkS.gbr   Top silkscreen
  {BOARD}-F_Paste.gbr   Top paste (SMT only on PCB)
  {BOARD}-B_Cu.gbr      Bottom copper
  {BOARD}-Edge_Cuts.gbr Outline
  {BOARD}-PTH.drl       Plated drills

PCB: 2 layer, 1.6 mm, HASL or ENIG
SMT assembly: only SMT parts in BOM — hand-solder THT (U3,U4,J2,J3,J5,J6,J7)

Rev D PCB: 78x50 mm, 2-layer — compact layout (see docs/PCB_LAYOUT.md)
Separate nets: GND_ESP, GND_RADIO_A, GND_USB_B, GND_USB_C — see docs/ISOLATION.md

Generated by: kicad/generate_gerbers.py
""",
        encoding="utf-8",
    )

    files = sorted(
        {f for f in args.output.iterdir() if f.is_file() and f.suffix in (".gbr", ".drl", ".txt")}
    )
    print(f"Exported {len(files)} files to {args.output}")
    for f in files:
        if f.is_file():
            print(f"  {f.name}")

    if args.zip:
        zip_path = args.output.parent / f"{BOARD}-jlc.zip"
        with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
            for f in args.output.iterdir():
                if f.is_file() and f.suffix in (".gbr", ".drl", ".txt"):
                    zf.write(f, f.name)
        print(f"ZIP: {zip_path} ({zip_path.stat().st_size // 1024} KiB)")

    if args.preview:
        import subprocess
        script = ROOT / "render_pcb_preview.py"
        subprocess.run([sys.executable, str(script), "--copy-docs"], check=True)


if __name__ == "__main__":
    main()
