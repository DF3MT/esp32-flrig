# ESP32-FLRIG Interface Shield (PCB)

Einsteck-Shield für **ESP32-2432S028 (CYD)** mit:

- **TXS0102** — CAT 3,3 V ↔ 5 V TTL  
- **PCM5102 + INMP441** — I2S Audio (48 kHz, wie Firmware)  
- **RJ45** — ein Kabel für **+5 V**, **CAT**, **Line Audio** ([Pinbelegung](docs/RJ45_PINOUT.md))

## Schnellstart

| Schritt | Dokument |
|---------|----------|
| 1. Gerber bei JLCPCB bestellen | [docs/JLCPCB_ORDER.md](docs/JLCPCB_ORDER.md) |
| 2. Platine löten | [docs/ASSEMBLY.md](docs/ASSEMBLY.md) |
| 3. Y-Kabel zum Funk | [docs/CABLE_ASSEMBLY.md](docs/CABLE_ASSEMBLY.md) |
| 4. Schaltplan verstehen | [docs/SCHEMATIC.md](docs/SCHEMATIC.md) |

![Blockdiagramm](assets/block_diagram.svg)

## Fertige Gerber für JLCPCB

**Upload:** [`fabrication/esp32-flrig-shield-jlc.zip`](fabrication/esp32-flrig-shield-jlc.zip)

## KiCad / Neu-Export

```bash
cd kicad
./export_gerbers.sh          # KiCad-CLI oder Python-Fallback
python3 generate_gerbers.py --zip
```

**Vor Produktion:** DRC in KiCad, Footprints mit Datenblatt abgleichen, ggf. Leiterbahnen nachziehen (Rev A enthält Basis-Layout + GND-Fläche).

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
