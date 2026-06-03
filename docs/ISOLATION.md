# Galvanic isolation between transceivers

[Deutsch](#deutsch) · [English](#english)

Switching between radios must **not** create a shared, low-impedance ground path between rigs (ground loops, hum, CAT errors). Shield **Rev C** uses **separate ground domains** linked only through isolators.

<a id="deutsch"></a>

## Deutsch

### Problem

Ohne Trennung sind **ESP32-Masse**, **Funk-A-Masse** (RJ45), **USB-B-Masse** und **USB-C-Masse** über Kabel und Shield verbunden → **Brummen**, falsche CAT-Bytes, USB-Abbrüche beim Wechsel zwischen mehreren Geräten.

### Lösung (Rev C)

| Pfad | CAT (Computer Aided Transceiver) | Audio | Masse |
|------|----------------------------------|-------|--------|
| **Funk A** (RJ45) | **U1 ISO7741** (UART digital isolator) | **T1/T2** Audio-Trafos 1:1 | **GND_RADIO_A** ≠ GND_ESP |
| **Funk B** (J5) | **U6 ADuM4160** (USB isolator) | im USB (UAC) | **GND_USB_B** |
| **Funk C** (J6) | **U7 ADuM4160** | im USB (UAC) | **GND_USB_C** |
| **ESP32-S3** | — | I2S nur ESP-Seite der Trafos | **GND_ESP** |

- **Kein** direkter Kupferverbund zwischen `GND_RADIO_A`, `GND_USB_B`, `GND_USB_C` und `GND_ESP`.
- **USB2422 (U5)** sitzt nur auf der **ESP-Seite**; je Downstream-Port ein **eigener ADuM4160** + **isoliertes DC/DC (U8/U9)** für VBUS.
- RJ45 **Pin 2/7** → `GND_RADIO_A` (nicht ESP-Masse). Gehäuse über **RC** (z. B. 1 nF + 1 MΩ) optional zu Funk-Masse, nicht hart zu ESP.
- **+5 V**: getrennte Versorgung empfohlen — ESP über eigenes USB/WiFi-Board; Funk A +5 V nur für isolierte Sekundärseite / Trafos.

### Wechsel zwischen Funkgeräten

Firmware schaltet **logisch** (rigctld-Kanal, USB-Index). **Elektrisch** bleiben die Massefelder getrennt — kein Relais, das GNDs zusammenlegt.

### Dokumentation Hardware

- [SCHEMATIC.md](../hardware/esp32-flrig-shield/docs/SCHEMATIC.md) (Rev C Netze)
- [ASSEMBLY.md](../hardware/esp32-flrig-shield/docs/ASSEMBLY.md) (Prüfpunkte)

<a id="english"></a>

## English

### Issue

Without isolation, **ESP ground**, **radio A ground** (RJ45), and **USB B/C grounds** are tied through cables and shields → **ground loops**, hum, CAT corruption when multiple rigs are connected.

### Solution (Rev C)

| Path | CAT (Computer Aided Transceiver) | Audio | Ground |
|------|----------------------------------|-------|--------|
| **Radio A** (RJ45) | **U1 ISO7741** digital isolator | **T1/T2** 1:1 audio transformers | **GND_RADIO_A** ≠ GND_ESP |
| **Radio B** (J5) | **U6 ADuM4160** USB isolator | via USB UAC | **GND_USB_B** |
| **Radio C** (J6) | **U7 ADuM4160** | via USB UAC | **GND_USB_C** |
| **ESP32-S3** | — | I2S on ESP side of transformers only | **GND_ESP** |

- **No** direct copper between radio/USB grounds and ESP ground.
- **USB2422** on ESP side only; **one ADuM4160 per** USB-A port plus **isolated DC/DC** for VBUS.
- RJ45 pins **2/7** → `GND_RADIO_A` only.
- Prefer **separate** +5 V feeds: ESP from its module; radio A +5 V for isolated secondary only.

### CAT acronym

**CAT** = **Computer Aided Transceiver** (ICOM/Yaesu control protocol), not related to any construction-equipment brand.
