#!/usr/bin/env python3
"""Repair nested [text](url) inside Markdown link URLs (linker accident)."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Nested link inside a URL path, e.g. wiki/[USB](https://.../Universal_Serial_Bus)-Audioklasse
NESTED_IN_URL = re.compile(
    r"(?<=[/:])\[([^\]]+)\]\((https?://[^)]+)\)([^)\]]*)"
)


def fix(text: str) -> str:
    prev = None
    while prev != text:
        prev = text
        text = NESTED_IN_URL.sub(_replace, text)
    return text


def _replace(m: re.Match[str]) -> str:
    term, url, suffix = m.group(1), m.group(2), m.group(3)
    if "/wiki/" in url:
        base, _path = url.split("/wiki/", 1)
        if suffix.startswith("-"):
            slug = f"{term}{suffix}"
            return f"{base}/wiki/{slug}"
        return url + suffix
    return url + suffix


def main() -> None:
    for path in ROOT.rglob("*.md"):
        if ".pio" in path.parts:
            continue
        raw = path.read_text(encoding="utf-8")
        fixed = fix(raw)
        if fixed != raw:
            path.write_text(fixed, encoding="utf-8")
            print("fixed", path.relative_to(ROOT))


if __name__ == "__main__":
    main()
