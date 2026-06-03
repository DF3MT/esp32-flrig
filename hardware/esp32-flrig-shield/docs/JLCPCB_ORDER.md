# Bestellung bei JLCPCB

**PCB Revision:** **C** (100×72 mm) — RJ45 + **2× USB-A** + **galvanische Trennung** (ISO7741, ADuM4160×2, Audio-Trafos)

## 1. Fertige Gerber (sofort bei JLCPCB hochladen)

**ZIP:** [`../fabrication/esp32-flrig-shield-jlc.zip`](../fabrication/esp32-flrig-shield-jlc.zip)

Enthält (KiCad-Namen, JLC-kompatibel):

| Datei | Layer |
|-------|--------|
| `esp32-flrig-shield-F_Cu.gbr` | Top Copper |
| `esp32-flrig-shield-F_Mask.gbr` | Top Mask |
| `esp32-flrig-shield-F_SilkS.gbr` | Top Silk |
| `esp32-flrig-shield-F_Paste.gbr` | Top Paste |
| `esp32-flrig-shield-B_Cu.gbr` | Bottom Copper |
| `esp32-flrig-shield-Edge_Cuts.gbr` | Outline |
| `esp32-flrig-shield-PTH.drl` | Bohrungen |

Einzeldateien: `fabrication/gerbers/`

### Neu erzeugen

```bash
cd hardware/esp32-flrig-shield/kicad
./export_gerbers.sh
# oder: python3 generate_gerbers.py --zip --preview
```

**Vorschau (JPG):** [`../fabrication/esp32-flrig-shield-preview.jpg`](../fabrication/esp32-flrig-shield-preview.jpg) — alle Gerber-Layer (Kupfer, Mask, Paste, Silk, Bohrungen).

```bash
python3 render_pcb_preview.py --copy-docs   # JPG + SVG, optional in docs/assets/
```

Mit **KiCad 7+** (`kicad-cli`) falls installiert, sonst automatisch **Python** (`kiutils` + `gerbonara` aus Projekt-`.venv`).

Alternativ in KiCad GUI: **Datei → Plot** → Gerber + Excellon → ZIP.

## 2. JLCPCB Parameter (empfohlen)

| Parameter | Wert |
|-----------|------|
| Layers | 2 |
| Material | FR-4 |
| Thickness | 1,6 mm |
| Copper | 1 oz |
| Surface | HASL (leadfree) oder ENIG |
| Min track/spacing | ≥ 6 mil (Design: 8 mil) |
| PCB Color | Schwarz / Grün nach Geschmack |

## 3. SMT Assembly

- **Top side only** (Komponenten laut CPL).
- **Bauteile ohne LCSC** in JLC-Editor als „Do not assemble“ markieren: U3, U4, J2 (Module/Header).
- `fabrication/BOM_JLCPCB.csv` hochladen, Footprints mit LCSC-Nummern abgleichen.
- **DNP (Do Not Place)** für optionale USB-C-Bestückung falls nicht in BOM.

### LCSC Referenzen (Prüfstand 2025 — vor Bestellung verifizieren)

| Ref | Teil | LCSC (Beispiel) |
|-----|------|-----------------|
| U1 | TXS0102DCUR | C15029 |
| U2 | AMS1117-3.3 | C6186 |
| F1 | 500mA PTC | C13685 |
| D1 | SS34 | C8678 |
| C* | 100nF 0805 | C496 |
| R* | 10k/1k 0805 | C25744 / C11702 |
| J1 | RJ45 8P8C | C124375 / C27168 |

## 4. Checkliste vor „Add to Cart“

- [ ] Gerber in JLC Viewer geöffnet — RJ45 und J2 Lochbilder korrekt
- [ ] BOM-Menge = CPL-Menge
- [ ] Shields mit **Stiftleiste** selbst löten (J2)
- [ ] I2S-Module **nach** SMT handlöten
- [ ] Firmware `include/config.h` GPIOs unverändert zu J2-Tabelle

## 5. Kosten-Richtwert

~5–15 € für 5 PCBs (ohne Versand); +3–8 € SMT pro Board bei nur Basic Parts.

## 6. Nach Lieferung

→ [ASSEMBLY.md](ASSEMBLY.md) und [RJ45_PINOUT.md](RJ45_PINOUT.md)
