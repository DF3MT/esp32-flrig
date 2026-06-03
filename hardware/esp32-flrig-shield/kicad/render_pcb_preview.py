#!/usr/bin/env python3
"""
Render PCB preview image (SVG + JPG) from Gerber export or KiCad PCB.

Uses gerbonara ``to_svg()`` with solid layer colors (no SVG filter effects).
``to_pretty_svg()`` looks nicer in Inkscape but renders blank in CairoSVG/browsers
because filter stacks are not supported for rasterization.

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
from gerbonara.utils import MM, Tag, setup_svg

ROOT = Path(__file__).resolve().parent
FAB = ROOT.parent / "fabrication"
GERBERS = FAB / "gerbers"
DEFAULT_JPG = FAB / "esp32-flrig-shield-preview.jpg"
DEFAULT_SVG = FAB / "esp32-flrig-shield-preview.svg"
DOCS_JPG = ROOT.parent.parent.parent / "docs" / "assets" / "pcb-preview.jpg"

# Bottom → top draw order (first = back)
DRAW_ORDER: list[tuple[str, str]] = [
    ("top", "copper"),
    ("top", "mask"),
    ("top", "paste"),
    ("top", "silk"),
    ("bottom", "copper"),
    ("mechanical", "outline"),
]

# Solid fills — must use explicit keys (defaultdict.get skips missing keys in gerbonara)
PREVIEW_COLORS: dict[str, str] = {
    "top copper": "#C87533",
    "top mask": "#1A5C1A",
    "top paste": "#888888",
    "top silk": "#E8E8E8",
    "bottom copper": "#C87533",
    "mechanical outline": "#D4A017",
    "drill pth": "#252525",
    "drill npth": "#505050",
    "drill unknown": "#303030",
}


def load_stack(from_gerbers: Path | None) -> LayerStack:
    if from_gerbers:
        return LayerStack.open_dir(from_gerbers)
    sys.path.insert(0, str(ROOT))
    from generate_gerbers import build_stack  # noqa: WPS433

    return build_stack()


def write_flat_svg(stack: LayerStack, path: Path, margin: float = 3) -> None:
    """Filter-free SVG suitable for browsers, CairoSVG, and README embeds."""
    bounds = stack.bounding_box(MM, default=((0, 0), (0, 0)))
    stroke_attrs = {"stroke_linejoin": "round", "stroke_linecap": "round"}
    layer_transform = f"translate(0 {bounds[0][1] + bounds[1][1]}) scale(1 -1)"

    tags: list[Tag] = []
    for side, use in DRAW_ORDER:
        key = (side, use)
        if key not in stack.graphic_layers:
            continue
        color = PREVIEW_COLORS.get(f"{side} {use}")
        if not color:
            continue
        layer = stack.graphic_layers[key]
        tags.append(
            Tag(
                "g",
                list(layer.svg_objects(svg_unit=MM, fg=color, bg="white", tag=Tag)),
                id=f"l-{side}-{use}",
                transform=layer_transform,
                **stroke_attrs,
            )
        )

    if stack.drill_pth and (color := PREVIEW_COLORS.get("drill pth")):
        tags.append(
            Tag(
                "g",
                list(stack.drill_pth.svg_objects(svg_unit=MM, fg=color, bg="white", tag=Tag)),
                id="l-drill-pth",
                transform=layer_transform,
                **stroke_attrs,
            )
        )
    if stack.drill_npth and (color := PREVIEW_COLORS.get("drill npth")):
        tags.append(
            Tag(
                "g",
                list(stack.drill_npth.svg_objects(svg_unit=MM, fg=color, bg="white", tag=Tag)),
                id="l-drill-npth",
                transform=layer_transform,
                **stroke_attrs,
            )
        )
    for i, layer in enumerate(stack._drill_layers):
        if color := PREVIEW_COLORS.get("drill unknown"):
            tags.append(
                Tag(
                    "g",
                    list(layer.svg_objects(svg_unit=MM, fg=color, bg="white", tag=Tag)),
                    id=f"l-drill-{i}",
                    transform=layer_transform,
                    **stroke_attrs,
                )
            )

    svg = setup_svg(
        tags,
        bounds,
        margin=margin,
        arg_unit=MM,
        svg_unit=MM,
        tag=Tag,
        pagecolor="#f4f4f4",
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
