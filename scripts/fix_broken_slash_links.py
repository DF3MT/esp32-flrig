#!/usr/bin/env python3
"""Fix /https://... artifacts from glossary linker (e.g. FT8/WSJT-X)."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

REPLACEMENTS: list[tuple[str, str]] = [
    # FT8 / WSJT-X
    (
        r"\[FT8\]\((https://[^)]+)\)/https://physics\.princeton\.edu/pulsar/k1jt/wsjtx\.html",
        r"[FT8](\1)/[WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html)",
    ),
    # Xiegu models
    (
        r"\[X6100\]\((https://www\.xiegu\.com/)\)/https://www\.xiegu\.com/",
        r"[X6100](\1)/[X6200](https://www.xiegu.com/)",
    ),
    # THT / SMT
    (
        r"\[THT\]\((https://de\.wikipedia\.org/wiki/Lochleiterplatte)\)/https://de\.wikipedia\.org/wiki/Oberfl%C3%A4chenmontage",
        r"[THT](\1)/[SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage)",
    ),
    # Icom / Yaesu slash typos
    (
        r"\[CAT\]\((https://en\.wikipedia\.org/wiki/Computer_Aided_Transceiver)\)/Icom/https://de\.wikipedia\.org/wiki/Yaesu",
        r"[CAT](\1)/[ICOM](https://de.wikipedia.org/wiki/Icom)/[Yaesu](https://de.wikipedia.org/wiki/Yaesu)",
    ),
    (
        r"\[CAT\]\((https://en\.wikipedia\.org/wiki/Computer_Aided_Transceiver)\)/https://physics\.princeton\.edu/pulsar/k1jt/wsjtx\.html",
        r"[CAT](\1)/[WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html)",
    ),
    (
        r"\[CI-V\]\((https://de\.wikipedia\.org/wiki/CI-V)\)/https://en\.wikipedia\.org/wiki/Computer_Aided_Transceiver",
        r"[CI-V](\1)/[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)",
    ),
    (
        r"Display/https://de\.wikipedia\.org/wiki/Universal_Serial_Bus",
        r"Display/[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)",
    ),
    (
        r"\[flrig\]\((https://github\.com/w1hkj/flrig)\)/https://en\.wikipedia\.org/wiki/Hamlib",
        r"[flrig](\1)/[Hamlib](https://en.wikipedia.org/wiki/Hamlib)",
    ),
    (
        r"\[USB\]\((https://de\.wikipedia\.org/wiki/Universal_Serial_Bus)\)/https://de\.wikipedia\.org/wiki/RJ-45",
        r"[USB](\1)/[RJ45](https://de.wikipedia.org/wiki/RJ-45)",
    ),
    (
        r"\[USB\]\((https://de\.wikipedia\.org/wiki/Universal_Serial_Bus)\)/https://de\.wikipedia\.org/wiki/WLAN",
        r"[USB](\1)/[WiFi](https://de.wikipedia.org/wiki/WLAN)",
    ),
    (
        r"radio/https://en\.wikipedia\.org/wiki/USB",
        r"radio/[USB](https://en.wikipedia.org/wiki/USB)",
    ),
    (
        r"\[ICOM\]\((https://en\.wikipedia\.org/wiki/Icom)\)/https://en\.wikipedia\.org/wiki/Yaesu_\(brand\)",
        r"[ICOM](\1)/[Yaesu](https://en.wikipedia.org/wiki/Yaesu_(brand))",
    ),
    (
        r"\[ESP32-S3\]\((https://www\.espressif\.com/en/products/socs/esp32-s3)\)/https://github\.com/witnessmenow/ESP32-Cheap-Yellow-Display",
        r"[ESP32-S3](\1)/[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)",
    ),
    # Double URL in link target
    (
        r"https://de\.wikipedia\.org/wiki/https://de\.wikipedia\.org/wiki/([^)\s]+)",
        r"https://de.wikipedia.org/wiki/\1",
    ),
    (
        r"https://en\.wikipedia\.org/wiki/https://en\.wikipedia\.org/wiki/([^)\s]+)",
        r"https://en.wikipedia.org/wiki/\1",
    ),
    (
        r"https://de\.wikipedia\.org/wiki/https://de\.wikipedia\.org/wiki/USB-Hub",
        r"https://de.wikipedia.org/wiki/USB-Hub",
    ),
]


def main() -> None:
    for path in ROOT.rglob("*.md"):
        if ".pio" in path.parts:
            continue
        text = path.read_text(encoding="utf-8")
        orig = text
        for pat, repl in REPLACEMENTS:
            text = re.sub(pat, repl, text)
        if text != orig:
            path.write_text(text, encoding="utf-8")
            print("fixed", path.relative_to(ROOT))


if __name__ == "__main__":
    main()
