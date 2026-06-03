#!/usr/bin/env python3
"""
Render PCB preview image (SVG + JPG) from Gerber export or KiCad PCB.

Preview stack (bottom → top), filter-free for CairoSVG/browsers:
  1. Copper (pours, tracks, pads)
  2. Green solder mask over the board outline
  3. Mask openings re-drawn in copper (exposed pads)
  4. Paste, silk, drills
  5. Board edge stroke (not a filled yellow rectangle)

Usage:
  python3 render_pcb_preview.py
  python3 render_pcb_preview.py --width 3600 --copy-docs
  python3 render_pcb_preview.py --from-gerbers ../fabrication/gerbers
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from gerbonara.layers import LayerStack
from gerbonara.utils import MM, Tag, setup_svg

ROOT = Path(__file__).resolve().parent
FAB = ROOT.parent / "fabrication"
GERBERS = FAB / "gerbers"
DEFAULT_JPG = FAB / "esp32-flrig-shield-preview.jpg"
DEFAULT_SVG = FAB / "esp32-flrig-shield-preview.svg"
DOCS_JPG = ROOT.parent.parent.parent / "docs" / "assets" / "pcb-preview.jpg"

COLOR_COPPER = "#C87533"
COLOR_MASK = "#1A5C1A"
COLOR_PASTE = "#888888"
COLOR_SILK = "#E8E8E8"
COLOR_EDGE = "#D4A017"
COLOR_DRILL = "#252525"
EDGE_STROKE_MM = 0.2


def load_stack(from_gerbers: Path | None) -> LayerStack:
    if from_gerbers:
        return LayerStack.open_dir(from_gerbers)
    sys.path.insert(0, str(ROOT))
    from generate_gerbers import build_stack  # noqa: WPS433

    return build_stack()


def _board_bounds(stack: LayerStack) -> tuple[float, float, float, float]:
    bb = stack.board_bounds(MM)
    if bb:
        return bb[0][0], bb[0][1], bb[1][0], bb[1][1]
    bb = stack.bounding_box(MM, default=((5, 5), (105, 72)))
    return bb[0][0], bb[0][1], bb[1][0], bb[1][1]


def _rect_path(x1: float, y1: float, x2: float, y2: float) -> str:
    return f"M {x1} {y1} L {x2} {y1} L {x2} {y2} L {x1} {y2} Z"


def _layer_group(stack: LayerStack, key: tuple[str, str], fg: str, transform: str) -> Tag | None:
    if key not in stack.graphic_layers:
        return None
    layer = stack.graphic_layers[key]
    objs = list(layer.svg_objects(svg_unit=MM, fg=fg, bg="white", tag=Tag))
    if not objs:
        return None
    return Tag(
        "g",
        objs,
        id=f"l-{'-'.join(key)}",
        transform=transform,
        stroke_linejoin="round",
        stroke_linecap="round",
    )


def write_flat_svg(stack: LayerStack, path: Path, margin: float = 3) -> None:
    bounds = stack.bounding_box(MM, default=((0, 0), (0, 0)))
    x1, y1, x2, y2 = _board_bounds(stack)
    layer_transform = f"translate(0 {bounds[0][1] + bounds[1][1]}) scale(1 -1)"
    board_path = _rect_path(x1, y1, x2, y2)

    tags: list[Tag] = []

    # 1 — copper (includes GND pour; mostly hidden by mask next)
    if g := _layer_group(stack, ("top", "copper"), COLOR_COPPER, layer_transform):
        tags.append(g)

    # 2 — solder mask over entire board
    tags.append(
        Tag(
            "path",
            [],
            id="l-soldermask-fill",
            d=board_path,
            fill=COLOR_MASK,
            stroke="none",
            transform=layer_transform,
        )
    )

    # 3 — mask openings (Gerber = clearances) shown as copper pads
    if ("top", "mask") in stack.graphic_layers:
        mask = stack.graphic_layers[("top", "mask")]
        openings = list(mask.svg_objects(svg_unit=MM, fg=COLOR_COPPER, bg="white", tag=Tag))
        if openings:
            tags.append(
                Tag(
                    "g",
                    openings,
                    id="l-mask-openings",
                    transform=layer_transform,
                    stroke_linejoin="round",
                    stroke_linecap="round",
                )
            )

    # 4 — paste & silk
    if g := _layer_group(stack, ("top", "paste"), COLOR_PASTE, layer_transform):
        tags.append(g)
    if g := _layer_group(stack, ("top", "silk"), COLOR_SILK, layer_transform):
        tags.append(g)

    # 5 — drills
    if stack.drill_pth and (color := COLOR_DRILL):
        tags.append(
            Tag(
                "g",
                list(stack.drill_pth.svg_objects(svg_unit=MM, fg=color, bg="white", tag=Tag)),
                id="l-drill-pth",
                transform=layer_transform,
                stroke_linejoin="round",
                stroke_linecap="round",
            )
        )

    # 6 — board edge (stroke only — never fill outline layer)
    tags.append(
        Tag(
            "path",
            [],
            id="l-board-edge",
            d=board_path,
            fill="none",
            stroke=COLOR_EDGE,
            stroke_width=EDGE_STROKE_MM,
            transform=layer_transform,
        )
    )

    svg = setup_svg(
        tags,
        bounds,
        margin=margin,
        arg_unit=MM,
        svg_unit=MM,
        tag=Tag,
        pagecolor="#e8e8e8",
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    if hasattr(svg, "write_to"):
        svg.write_to(path)
    else:
        path.write_text(str(svg))


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
    ap.add_argument("--margin", type=float, default=3, help="Margin around board (mm)")
    args = ap.parse_args()

    gerber_dir = args.from_gerbers
    if gerber_dir is None and not (ROOT / "esp32-flrig-shield.kicad_pcb").exists():
        gerber_dir = GERBERS

    stack = load_stack(gerber_dir)
    stack.merge_drill_layers()

    write_flat_svg(stack, args.svg, margin=args.margin)
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
