#!/usr/bin/env python3
"""Link technical terms in Markdown to context-appropriate sources (idempotent)."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Longer phrases first. URL = None → skip.
DE_TERMS: list[tuple[str, str | None]] = [
    ("Computer Aided Transceiver", "https://en.wikipedia.org/wiki/Computer_Aided_Transceiver"),
    ("Cheap Yellow Display", "https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display"),
    ("Galvanische Trennung", "https://de.wikipedia.org/wiki/Galvanische_Trennung"),
    ("galvanisch getrennt", "https://de.wikipedia.org/wiki/Galvanische_Trennung"),
    ("galvanisch", "https://de.wikipedia.org/wiki/Galvanische_Trennung"),
    ("Yaesu CAT", "https://en.wikipedia.org/wiki/Computer_Aided_Transceiver"),
    ("USB-On-The-Go", "https://de.wikipedia.org/wiki/USB_On-The-Go"),
    ("USB OTG", "https://de.wikipedia.org/wiki/USB_On-The-Go"),
    ("USB-2.0", "https://de.wikipedia.org/wiki/USB"),
    ("USB 2.0", "https://de.wikipedia.org/wiki/USB"),
    ("USB-UAC", "https://de.wikipedia.org/wiki/USB-Audioklasse"),
    ("USB UAC", "https://de.wikipedia.org/wiki/USB-Audioklasse"),
    ("USB-Audio", "https://de.wikipedia.org/wiki/USB-Audioklasse"),
    ("USB-CDC", "https://de.wikipedia.org/wiki/USB_Communications_Device_Class"),
    ("USB CDC", "https://de.wikipedia.org/wiki/USB_Communications_Device_Class"),
    ("USB-Host", "https://de.wikipedia.org/wiki/USB"),
    ("USB Host", "https://de.wikipedia.org/wiki/USB"),
    ("USB-Hub", "https://de.wikipedia.org/wiki/USB-Hub"),
    ("USB Hub", "https://de.wikipedia.org/wiki/USB-Hub"),
    ("USB-A", "https://de.wikipedia.org/wiki/USB"),
    ("USB-Kabel", "https://de.wikipedia.org/wiki/USB"),
    ("USB-Serial", "https://de.wikipedia.org/wiki/USB_Communications_Device_Class"),
    ("Level-Shifter", "https://de.wikipedia.org/wiki/Pegelwandler"),
    ("Pegelwandler", "https://de.wikipedia.org/wiki/Pegelwandler"),
    ("Bidirectional-Level-Shifter", "https://de.wikipedia.org/wiki/Pegelwandler"),
    ("Open-Collector", "https://de.wikipedia.org/wiki/Offener_Kollektor"),
    ("Open Collector", "https://de.wikipedia.org/wiki/Offener_Kollektor"),
    ("Audio-Trafos", "https://de.wikipedia.org/wiki/Transformator"),
    ("Audio-Trafo", "https://de.wikipedia.org/wiki/Transformator"),
    ("Audio-Bridge", None),  # project-specific
    ("ESP32-S3", "https://www.espressif.com/en/products/socs/esp32-s3"),
    ("ESP32", "https://de.wikipedia.org/wiki/ESP32"),
    ("PlatformIO", "https://platformio.org/"),
    ("WSJT-X", "https://physics.princeton.edu/pulsar/k1jt/wsjtx.html"),
    ("rotctld", "https://hamlib.sourceforge.net/html/rotctld.1.html"),
    ("rigctld", "https://hamlib.sourceforge.net/html/rigctld.1.html"),
    ("rotctl", "https://hamlib.sourceforge.net/html/rotctl.1.html"),
    ("rigctl", "https://hamlib.sourceforge.net/html/rigctl.1.html"),
    ("Hamlib", "https://en.wikipedia.org/wiki/Hamlib"),
    ("VB-Cable", "https://vb-audio.com/Cable/"),
    ("VB-Audio", "https://vb-audio.com/Cable/"),
    ("ISO7741", "https://www.ti.com/product/ISO7741"),
    ("ADuM4160", "https://www.analog.com/en/products/adum4160.html"),
    ("USB2422", "https://www.microchip.com/en-us/product/USB2422"),
    ("PCM5102A", "https://www.ti.com/product/PCM5102"),
    ("PCM5102", "https://www.ti.com/product/PCM5102"),
    ("INMP441", "https://www.invensense.com/products/digital/inmp441/"),
    ("TXS0108E", "https://www.ti.com/product/TXS0108E"),
    ("TXS0102", "https://www.ti.com/product/TXS0102"),
    ("MAX3232", "https://de.wikipedia.org/wiki/MAX3232"),
    ("MAX98357A", "https://www.analog.com/en/products/max98357a.html"),
    ("AMS1117", "https://en.wikipedia.org/wiki/List_of_linear_integrated_circuits"),
    ("B0505S", "https://en.wikipedia.org/wiki/DC-to-DC_converter"),
    ("CH340", "https://de.wikipedia.org/wiki/USB-UART-Bridge"),
    ("AliExpress", "https://de.wikipedia.org/wiki/AliExpress"),
    ("PulseAudio", "https://de.wikipedia.org/wiki/PulseAudio"),
    ("Potentiometer", "https://de.wikipedia.org/wiki/Potentiometer"),
    ("Potentiometern", "https://de.wikipedia.org/wiki/Potentiometer"),
    ("Potis", "https://de.wikipedia.org/wiki/Potentiometer"),
    ("Freilaufdiode", "https://de.wikipedia.org/wiki/Freilaufdiode"),
    ("Brumschleife", "https://de.wikipedia.org/wiki/Brumschleife"),
    ("Ground-Loop", "https://de.wikipedia.org/wiki/Brumschleife"),
    ("Stückliste", "https://de.wikipedia.org/wiki/St%C3%BCckliste"),
    ("Leiterplatte", "https://de.wikipedia.org/wiki/Leiterplatte"),
    ("Amateurfunkgerät", "https://de.wikipedia.org/wiki/Amateurfunk"),
    ("Amateurfunk", "https://de.wikipedia.org/wiki/Amateurfunk"),
    ("Funkgeräte", "https://de.wikipedia.org/wiki/Transceiver"),
    ("Funkgerät", "https://de.wikipedia.org/wiki/Transceiver"),
    ("Funkgeräten", "https://de.wikipedia.org/wiki/Transceiver"),
    ("Funkamateur", "https://de.wikipedia.org/wiki/Amateurfunk"),
    ("Transceiver", "https://de.wikipedia.org/wiki/Transceiver"),
    ("Multimeter", "https://de.wikipedia.org/wiki/Multimeter"),
    ("Dupont", "https://de.wikipedia.org/wiki/Steckbrett#Steckbrücken"),
    ("Steckbrett", "https://de.wikipedia.org/wiki/Steckbrett"),
    ("Lötkolben", "https://de.wikipedia.org/wiki/L%C3%B6tkolben"),
    ("Schottky", "https://de.wikipedia.org/wiki/Schottky-Diode"),
    ("Polyfuse", "https://de.wikipedia.org/wiki/Polymer-Schmelzsicherung"),
    ("Elko", "https://de.wikipedia.org/wiki/Elektrolytkondensator"),
    ("Keramik", "https://de.wikipedia.org/wiki/Keramikkondensator"),
    ("WLAN", "https://de.wikipedia.org/wiki/WLAN"),
    ("WiFi", "https://de.wikipedia.org/wiki/WLAN"),
    ("DHCP", "https://de.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol"),
    ("SSID", "https://de.wikipedia.org/wiki/Service_Set_Identifier"),
    ("CI-V", "https://de.wikipedia.org/wiki/CI-V"),
    ("CAT", "https://en.wikipedia.org/wiki/Computer_Aided_Transceiver"),
    ("I2S", "https://de.wikipedia.org/wiki/I%C2%B2S"),
    ("UART", "https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle"),
    ("RJ45", "https://de.wikipedia.org/wiki/RJ-45"),
    ("GPIO", "https://de.wikipedia.org/wiki/General_Purpose_Input/Output"),
    ("UDP", "https://de.wikipedia.org/wiki/User_Datagram_Protocol"),
    ("TCP", "https://de.wikipedia.org/wiki/Transmission_Control_Protocol"),
    ("FT8/WSJT-X", "https://physics.princeton.edu/pulsar/k1jt/wsjtx.html"),
    ("FT8", "https://de.wikipedia.org/wiki/FT8_(Amateurfunk)"),
    ("flrig", "https://github.com/w1hkj/flrig"),
    ("fldigi", "https://de.wikipedia.org/wiki/Fldigi"),
    ("JLCPCB", "https://jlcpcb.com/"),
    ("KiCad", "https://de.wikipedia.org/wiki/KiCad"),
    ("Gerber", "https://en.wikipedia.org/wiki/Gerber_format"),
    ("Gerbern", "https://en.wikipedia.org/wiki/Gerber_format"),
    ("PCB", "https://de.wikipedia.org/wiki/Leiterplatte"),
    ("DRC", "https://en.wikipedia.org/wiki/Design_rule_check"),
    ("BOM", "https://de.wikipedia.org/wiki/St%C3%BCckliste"),
    ("CPL", "https://en.wikipedia.org/wiki/Pick-and-place"),
    ("SMT", "https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage"),
    ("THT", "https://de.wikipedia.org/wiki/Lochleiterplatte"),
    ("USB", "https://de.wikipedia.org/wiki/Universal_Serial_Bus"),
    ("TTL", "https://de.wikipedia.org/wiki/Transistor-Transistor-Logik"),
    ("RS-232", "https://de.wikipedia.org/wiki/RS-232"),
    ("PTT", "https://de.wikipedia.org/wiki/Sende-Empfang-Umschaltung"),
    ("ICOM", "https://de.wikipedia.org/wiki/Icom"),
    ("Yaesu", "https://de.wikipedia.org/wiki/Yaesu"),
    ("Xiegu", "https://www.xiegu.com/"),
    ("X6100/X6200", "https://www.xiegu.com/"),
    ("X6100", "https://www.xiegu.com/"),
    ("X6200", "https://www.xiegu.com/"),
    ("Rotor", "https://de.wikipedia.org/wiki/Rotor_(Antenne)"),
    ("CYD", "https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display"),
    ("TTGO", "https://en.wikipedia.org/wiki/LilyGo"),
    ("JSON", "https://de.wikipedia.org/wiki/JSON"),
    ("ALSA", "https://de.wikipedia.org/wiki/Advanced_Linux_Sound_Architecture"),
    ("WSL", "https://learn.microsoft.com/de-de/windows/wsl/"),
    ("Python", "https://de.wikipedia.org/wiki/Python_(Programmiersprache)"),
    ("HTML", "https://de.wikipedia.org/wiki/Hypertext_Markup_Language"),
    ("HTTP", "https://de.wikipedia.org/wiki/Hypertext_Transfer_Protocol"),
    ("Linux", "https://de.wikipedia.org/wiki/Linux"),
    ("Windows", "https://de.wikipedia.org/wiki/Microsoft_Windows"),
    ("pip", "https://de.wikipedia.org/wiki/Pip_(Python)"),
    ("NPN", "https://de.wikipedia.org/wiki/Bipolartransistor"),
    ("SSOP", "https://de.wikipedia.org/wiki/Small-outline_integrated_circuit"),
    ("SOIC", "https://de.wikipedia.org/wiki/Small-outline_integrated_circuit"),
    ("VBUS", "https://de.wikipedia.org/wiki/Universal_Serial_Bus"),
    ("bps", "https://de.wikipedia.org/wiki/Bit_pro_Sekunde"),
    ("kHz", "https://de.wikipedia.org/wiki/Hertz"),
    ("MHz", "https://de.wikipedia.org/wiki/Hertz"),
]

EN_TERMS: list[tuple[str, str | None]] = [
    ("Computer Aided Transceiver", "https://en.wikipedia.org/wiki/Computer_Aided_Transceiver"),
    ("Cheap Yellow Display", "https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display"),
    ("galvanically isolated", "https://en.wikipedia.org/wiki/Galvanic_isolation"),
    ("galvanic isolation", "https://en.wikipedia.org/wiki/Galvanic_isolation"),
    ("ground loops", "https://en.wikipedia.org/wiki/Ground_loop_(electricity)"),
    ("ground loop", "https://en.wikipedia.org/wiki/Ground_loop_(electricity)"),
    ("Yaesu CAT", "https://en.wikipedia.org/wiki/Computer_Aided_Transceiver"),
    ("USB On-The-Go", "https://en.wikipedia.org/wiki/USB_On-The-Go"),
    ("USB OTG", "https://en.wikipedia.org/wiki/USB_On-The-Go"),
    ("USB UAC", "https://en.wikipedia.org/wiki/USB_audio_device_class"),
    ("USB CDC", "https://en.wikipedia.org/wiki/USB_communications_device_class"),
    ("USB host", "https://en.wikipedia.org/wiki/USB"),
    ("USB Host", "https://en.wikipedia.org/wiki/USB"),
    ("USB hub", "https://en.wikipedia.org/wiki/USB_hub"),
    ("USB Hub", "https://en.wikipedia.org/wiki/USB_hub"),
    ("USB Type-A", "https://en.wikipedia.org/wiki/USB"),
    ("USB-A", "https://en.wikipedia.org/wiki/USB"),
    ("USB cable", "https://en.wikipedia.org/wiki/USB"),
    ("USB cables", "https://en.wikipedia.org/wiki/USB"),
    ("USB Serial", "https://en.wikipedia.org/wiki/USB_communications_device_class"),
    ("level shifter", "https://en.wikipedia.org/wiki/Logic_level#Level_shifting"),
    ("Level shifter", "https://en.wikipedia.org/wiki/Logic_level#Level_shifting"),
    ("open-collector", "https://en.wikipedia.org/wiki/Open_collector"),
    ("open collector", "https://en.wikipedia.org/wiki/Open_collector"),
    ("Open collector", "https://en.wikipedia.org/wiki/Open_collector"),
    ("audio transformers", "https://en.wikipedia.org/wiki/Transformer"),
    ("audio transformer", "https://en.wikipedia.org/wiki/Transformer"),
    ("ESP32-S3", "https://www.espressif.com/en/products/socs/esp32-s3"),
    ("ESP32", "https://en.wikipedia.org/wiki/ESP32"),
    ("PlatformIO", "https://platformio.org/"),
    ("WSJT-X", "https://physics.princeton.edu/pulsar/k1jt/wsjtx.html"),
    ("rotctld", "https://hamlib.sourceforge.net/html/rotctld.1.html"),
    ("rigctld", "https://hamlib.sourceforge.net/html/rigctld.1.html"),
    ("rotctl", "https://hamlib.sourceforge.net/html/rotctl.1.html"),
    ("rigctl", "https://hamlib.sourceforge.net/html/rigctl.1.html"),
    ("Hamlib", "https://en.wikipedia.org/wiki/Hamlib"),
    ("VB-Cable", "https://vb-audio.com/Cable/"),
    ("VB-Audio", "https://vb-audio.com/Cable/"),
    ("ISO7741", "https://www.ti.com/product/ISO7741"),
    ("ADuM4160", "https://www.analog.com/en/products/adum4160.html"),
    ("USB2422", "https://www.microchip.com/en-us/product/USB2422"),
    ("PCM5102", "https://www.ti.com/product/PCM5102"),
    ("INMP441", "https://www.invensense.com/products/digital/inmp441/"),
    ("MAX3232", "https://en.wikipedia.org/wiki/MAX3232"),
    ("CH340", "https://en.wikipedia.org/wiki/USB_adapter"),
    ("PulseAudio", "https://en.wikipedia.org/wiki/PulseAudio"),
    ("flyback diode", "https://en.wikipedia.org/wiki/Flyback_diode"),
    ("Bill of materials", "https://en.wikipedia.org/wiki/Bill_of_materials"),
    ("printed circuit board", "https://en.wikipedia.org/wiki/Printed_circuit_board"),
    ("transceivers", "https://en.wikipedia.org/wiki/Transceiver"),
    ("transceiver", "https://en.wikipedia.org/wiki/Transceiver"),
    ("ham radio", "https://en.wikipedia.org/wiki/Amateur_radio"),
    ("WiFi", "https://en.wikipedia.org/wiki/Wi-Fi"),
    ("WLAN", "https://en.wikipedia.org/wiki/Wi-Fi"),
    ("DHCP", "https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol"),
    ("SSID", "https://en.wikipedia.org/wiki/Service_set_(802.11_network)"),
    ("CI-V", "https://en.wikipedia.org/wiki/CI-V"),
    ("CAT", "https://en.wikipedia.org/wiki/Computer_Aided_Transceiver"),
    ("I2S", "https://en.wikipedia.org/wiki/I%C2%B2S"),
    ("UART", "https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter"),
    ("RJ45", "https://en.wikipedia.org/wiki/Registered_jack"),
    ("GPIO", "https://en.wikipedia.org/wiki/General-purpose_input/output"),
    ("UDP", "https://en.wikipedia.org/wiki/User_Datagram_Protocol"),
    ("TCP", "https://en.wikipedia.org/wiki/Transmission_Control_Protocol"),
    ("FT8/WSJT-X", "https://physics.princeton.edu/pulsar/k1jt/wsjtx.html"),
    ("FT8", "https://en.wikipedia.org/wiki/FT8_(digital_mode)"),
    ("flrig", "https://github.com/w1hkj/flrig"),
    ("fldigi", "https://en.wikipedia.org/wiki/Fldigi"),
    ("JLCPCB", "https://jlcpcb.com/"),
    ("KiCad", "https://en.wikipedia.org/wiki/KiCad"),
    ("Gerber", "https://en.wikipedia.org/wiki/Gerber_format"),
    ("PCB", "https://en.wikipedia.org/wiki/Printed_circuit_board"),
    ("DRC", "https://en.wikipedia.org/wiki/Design_rule_check"),
    ("BOM", "https://en.wikipedia.org/wiki/Bill_of_materials"),
    ("CPL", "https://en.wikipedia.org/wiki/Pick-and-place"),
    ("SMT", "https://en.wikipedia.org/wiki/Surface-mount_technology"),
    ("USB", "https://en.wikipedia.org/wiki/USB"),
    ("TTL", "https://en.wikipedia.org/wiki/Transistor%E2%80%93transistor_logic"),
    ("RS-232", "https://en.wikipedia.org/wiki/RS-232"),
    ("PTT", "https://en.wikipedia.org/wiki/Push-to-talk"),
    ("ICOM", "https://en.wikipedia.org/wiki/Icom"),
    ("Yaesu", "https://en.wikipedia.org/wiki/Yaesu_(brand)"),
    ("Xiegu", "https://www.xiegu.com/"),
    ("rotor", "https://en.wikipedia.org/wiki/Antenna_rotator"),
    ("Rotor", "https://en.wikipedia.org/wiki/Antenna_rotator"),
    ("CYD", "https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display"),
    ("TTGO", "https://en.wikipedia.org/wiki/LilyGo"),
    ("JSON", "https://en.wikipedia.org/wiki/JSON"),
    ("ALSA", "https://en.wikipedia.org/wiki/Advanced_Linux_Sound_Architecture"),
    ("WSL", "https://learn.microsoft.com/en-us/windows/wsl/"),
    ("Python", "https://en.wikipedia.org/wiki/Python_(programming_language)"),
    ("Linux", "https://en.wikipedia.org/wiki/Linux"),
    ("Windows", "https://en.wikipedia.org/wiki/Microsoft_Windows"),
    ("pip", "https://en.wikipedia.org/wiki/Pip_(package_manager)"),
    ("NPN", "https://en.wikipedia.org/wiki/Bipolar_junction_transistor"),
    ("VBUS", "https://en.wikipedia.org/wiki/USB"),
]

SKIP_IN_CODE = True
LINKED = re.compile(r"\[[^\]]*\]\([^)]+\)")


def _protected_spans(text: str) -> list[tuple[int, int]]:
    spans: list[tuple[int, int]] = []
    # Full markdown links [text](url) — never relink inside targets
    for m in re.finditer(r"\[[^\]]*\]\([^)]*\)", text):
        spans.append((m.start(), m.end()))
    # Remaining partial links
    for m in LINKED.finditer(text):
        spans.append((m.start(), m.end()))
    # Inline code
    for m in re.finditer(r"`[^`]+`", text):
        spans.append((m.start(), m.end()))
    # URLs
    for m in re.finditer(r"https?://[^\s)>]+", text):
        spans.append((m.start(), m.end()))
    # Image/link targets in ![alt](url) already covered
    return sorted(spans)


def _in_span(pos: int, spans: list[tuple[int, int]]) -> bool:
    return any(a <= pos < b for a, b in spans)


def link_prose(text: str, terms: list[tuple[str, str | None]]) -> str:
    parts = text.split("```")
    for idx in range(0, len(parts), 2):
        chunk = parts[idx]
        spans = _protected_spans(chunk)
        for term, url in terms:
            if not url:
                continue
            pattern = re.compile(r"(?<!\[)(?<!\w)" + re.escape(term) + r"(?!\w)(?!\])")
            out = []
            last = 0
            for m in pattern.finditer(chunk):
                if _in_span(m.start(), spans):
                    out.append(chunk[last : m.end()])
                    last = m.end()
                    continue
                # Skip "A/B" style compounds (e.g. FT8/WSJT-X, X6100/X6200)
                after = chunk[m.end() : m.end() + 1]
                before = chunk[m.start() - 1 : m.start()] if m.start() else ""
                if after == "/" or before == "/":
                    out.append(chunk[last : m.end()])
                    last = m.end()
                    continue
                out.append(chunk[last : m.start()])
                repl = f"[{m.group(0)}]({url})"
                out.append(repl)
                delta = len(repl) - (m.end() - m.start())
                spans = [
                    (a + delta if a >= m.end() else a, b + delta if b >= m.end() else b)
                    for a, b in spans
                ]
                spans.append((m.start(), m.start() + len(repl)))
                last = m.end()
            out.append(chunk[last:])
            chunk = "".join(out)
        parts[idx] = chunk
    return "```".join(parts)


def process_file(path: Path, lang: str) -> bool:
    raw = path.read_text(encoding="utf-8")
    text = raw
    if lang == "both":
        m_de = re.search(r'<a id="deutsch"></a>', text)
        m_en = re.search(r'<a id="english"></a>', text)
        if m_de and m_en:
            text = (
                text[: m_de.start()]
                + link_prose(text[m_de.start() : m_en.start()], DE_TERMS)
                + link_prose(text[m_en.start() :], EN_TERMS)
            )
        elif m_de:
            text = text[: m_de.start()] + link_prose(text[m_de.start() :], DE_TERMS)
        elif m_en:
            text = text[: m_en.start()] + link_prose(text[m_en.start() :], EN_TERMS)
        else:
            text = link_prose(text, DE_TERMS)
    elif lang == "en":
        text = link_prose(text, EN_TERMS)
    else:
        text = link_prose(text, DE_TERMS)
    if text != raw:
        path.write_text(text, encoding="utf-8")
        return True
    return False


def main() -> int:
    files_lang = [
        (ROOT / "README.md", "both"),
        (ROOT / "docs/GLOSSARY.md", "both"),
        (ROOT / "docs/ISOLATION.md", "both"),
        (ROOT / "docs/MULTI_RADIO_DE.md", "de"),
        (ROOT / "docs/MULTI_RADIO_EN.md", "en"),
        (ROOT / "docs/DUAL_RADIO_DE.md", "de"),
        (ROOT / "docs/RADIOS.md", "de"),
        (ROOT / "docs/GUIDE_DE.md", "de"),
        (ROOT / "hardware/esp32-flrig-shield/README.md", "both"),
        (ROOT / "hardware/esp32-flrig-shield/docs/ASSEMBLY.md", "de"),
        (ROOT / "hardware/esp32-flrig-shield/docs/CABLE_ASSEMBLY.md", "de"),
        (ROOT / "hardware/esp32-flrig-shield/docs/JLCPCB_ORDER.md", "de"),
        (ROOT / "hardware/esp32-flrig-shield/docs/RJ45_PINOUT.md", "de"),
        (ROOT / "hardware/esp32-flrig-shield/docs/SCHEMATIC.md", "de"),
    ]
    changed = 0
    for path, lang in files_lang:
        if path.exists() and process_file(path, lang):
            print("updated", path.relative_to(ROOT))
            changed += 1
    print(f"done: {changed} file(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
