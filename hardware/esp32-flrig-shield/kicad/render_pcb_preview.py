#!/usr/bin/env python3
"""
Render PCB preview image (SVG + JPG) from Gerber export or KiCad PCB.

Uses gerbonara (same stack as generate_gerbers.py) with to_pretty_svg() so all
layers appear: top copper, mask, paste, silk, bottom copper, outline, drills.

Usage:
  python3 render_pcb_preview.py
  python3 render_pcb_preview.py --width 3200
  python3 render_pcb_preview.py --from-gerbers ../fabrication/gerbers
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from gerbonara.layers import LayerStack

ROOT = Path(__file__).resolve().parent
FAB = ROOT.parent / "fabrication"
GERBERS = FAB / "gerbers"
DEFAULT_JPG = FAB / "esp32-flrig-shield-preview.jpg"
DEFAULT_SVG = FAB / "esp32-flrig-shield-preview.svg"
DOCS_JPG = ROOT.parent.parent.parent / "docs" / "assets" / "pcb-preview.jpg"


def load_stack(from_gerbers: Path | None) -> LayerStack:
    if from_gerbers:
        return LayerStack.open_dir(from_gerbers)
    sys.path.insert(0, str(ROOT))
    from generate_gerbers import build_stack  # noqa: WPS433

    return build_stack()


def write_svg(stack: LayerStack, path: Path) -> None:
    pretty = stack.to_pretty_svg()
    path.parent.mkdir(parents=True, exist_ok=True)
    if hasattr(pretty, "write_to"):
        pretty.write_to(path)
    else:
        path.write_text(str(pretty))


def svg_to_jpg(svg_path: Path, jpg_path: Path, width: int, quality: int) -> None:
    import cairosvg
    from PIL import Image

    png_tmp = jpg_path.with_suffix(".png.tmp")
    cairosvg.svg2png(url=str(svg_path), write_to=str(png_tmp), output_width=width)
    Image.open(png_tmp).convert("RGB").save(
        jpg_path, "JPEG", quality=quality, optimize=True
    )
    png_tmp.unlink(missing_ok=True)


def main() -> None:
    ap = argparse.ArgumentParser(description="Render PCB preview JPG from Gerbers")
    ap.add_argument(
        "--from-gerbers",
        type=Path,
        default=None,
        help=f"Gerber directory (default: generate from {ROOT / 'esp32-flrig-shield.kicad_pcb'})",
    )
    ap.add_argument("-o", "--jpg", type=Path, default=DEFAULT_JPG)
    ap.add_argument("--svg", type=Path, default=DEFAULT_SVG)
    ap.add_argument("--copy-docs", action="store_true", help="Also write docs/assets/pcb-preview.jpg")
    ap.add_argument("--width", type=int, default=3200, help="Output image width in pixels")
    ap.add_argument("--quality", type=int, default=92, help="JPEG quality 1-100")
    args = ap.parse_args()

    gerber_dir = args.from_gerbers
    if gerber_dir is None and not (ROOT / "esp32-flrig-shield.kicad_pcb").exists():
        gerber_dir = GERBERS

    stack = load_stack(gerber_dir)
    stack.merge_drill_layers()

    write_svg(stack, args.svg)
    svg_to_jpg(args.svg, args.jpg, args.width, args.quality)

    if args.copy_docs:
        DOCS_JPG.parent.mkdir(parents=True, exist_ok=True)
        svg_to_jpg(args.svg, DOCS_JPG, args.width, args.quality)

    bb = stack.bounding_box()
    print(f"Board bounds (mm): {bb[0]} → {bb[1]}")
    print(f"SVG: {args.svg} ({args.svg.stat().st_size // 1024} KiB)")
    print(f"JPG: {args.jpg} ({args.jpg.stat().st_size // 1024} KiB, width={args.width}px)")
    if args.copy_docs:
        print(f"Docs: {DOCS_JPG}")


if __name__ == "__main__":
    main()
