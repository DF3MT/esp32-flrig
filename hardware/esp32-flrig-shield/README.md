# ESP32-FLRIG Interface Shield (PCB Rev C)

Shield für **ESP32-S3** (USB-Host) oder optional **CYD** (nur Kanal A):

| Anschluss | Funktion |
|-----------|----------|
| **J1 RJ45** | Funk **A**: +5 V, **analog CAT**, **Line Audio** |
| **J5 / J6** | **USB-A Buchse weiblich** — Funk **B** und **C** (CAT + Audio per USB) |
| **U5 USB2422** | 2-Port-Hub, Upstream **J3** → ESP32-S3 OTG |
| **J7 / J3** | **Rotor** (2× Taster, 2× Open-Collector) parallel zu allen Funkkanälen |
| **J2** | 2×10 für **CYD** (Kanal A + I2S, kein USB-Host) |

- **TXS0102** — CAT 3,3 V ↔ 5 V TTL (RJ45)  
- **PCM5102 + INMP441** — I2S Audio Funk A (48 kHz)  

**3× Transceiver + 1× Rotor** gleichzeitig über **WiFi** — **galvanisch getrennte** Masse pro Funk ([docs/ISOLATION.md](../../docs/ISOLATION.md)).

**CAT** = **Computer Aided Transceiver** (Steuerprotokoll).

## Schnellstart

| Schritt | Dokument |
|---------|----------|
| 1. Gerber bei JLCPCB bestellen | [docs/JLCPCB_ORDER.md](docs/JLCPCB_ORDER.md) |
| 2. Platine löten | [docs/ASSEMBLY.md](docs/ASSEMBLY.md) |
| 3. Y-Kabel zum Funk | [docs/CABLE_ASSEMBLY.md](docs/CABLE_ASSEMBLY.md) |
| 4. Schaltplan verstehen | [docs/SCHEMATIC.md](docs/SCHEMATIC.md) |

![Blockdiagramm](assets/block_diagram.svg)

![PCB preview (Gerber)](fabrication/esp32-flrig-shield-preview.jpg)

## Fertige Gerber für JLCPCB

**Upload:** [`fabrication/esp32-flrig-shield-jlc.zip`](fabrication/esp32-flrig-shield-jlc.zip)

## KiCad / Neu-Export

```bash
cd kicad
./export_gerbers.sh          # KiCad-CLI oder Python-Fallback
python3 generate_gerbers.py --zip
```

**Vor Produktion:** DRC in KiCad, USB2422 + USB-A Footprints mit Datenblatt prüfen (Rev **B**: 100×72 mm, Hub + 2× USB-A).

### Firmware (Rev B / 3 Funk)

```bash
pio run -t upload -e esp32-flrig-shield
```

## Dateien

```
esp32-flrig-shield/
├── kicad/           KiCad 7 Projekt
├── fabrication/     BOM, CPL, Gerber-ZIP (nach Export)
├── docs/            Bauanleitung, RJ45, JLCPCB
└── assets/          SVG-Diagramme
```

## Rotor-GPIO (optional am CYD-Header J2)

Nicht auf der RJ45-Leitung — direkt am **ESP32 CYD** (siehe [Haupt-README](../../README.md#4--rotor-hamlib-rotctld)):

| J2 / Funktion | GPIO (CYD) |
|---------------|------------|
| Taster Azimut CCW | 27 |
| Taster Azimut CW | 5 |
| Open Collector CCW | 18 |
| Open Collector CW | 19 |

Extern: NPN + Relais + Freilaufdiode pro Richtung. **rotctld** am ESP: Port **4535**.

Firmware & FT8: [Projekt-README](../../README.md)
