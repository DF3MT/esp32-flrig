# Bestellung bei [JLCPCB](https://jlcpcb.com/)

**[PCB](https://de.wikipedia.org/wiki/Leiterplatte) Revision:** **D** (**78×50 mm**, 2-layer) — [RJ45](https://de.wikipedia.org/wiki/RJ-45) + **2× [USB-A](https://de.wikipedia.org/wiki/USB)** · Layout: [PCB_LAYOUT.md](PCB_LAYOUT.md)

## 1. Fertige [Gerber](https://en.wikipedia.org/wiki/Gerber_format) (sofort bei [JLCPCB](https://jlcpcb.com/) hochladen)

**ZIP:** [`../fabrication/esp32-flrig-shield-jlc.zip`](../fabrication/esp32-flrig-shield-jlc.zip)

Enthält ([KiCad](https://de.wikipedia.org/wiki/KiCad)-Namen, JLC-kompatibel):

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
# oder: python3 generate_gerbers.py --validate --zip --preview
```

**Prüfung vor Upload:** `python3 generate_gerbers.py --validate` (Kupfer, Mask-Öffnungen, 4× Edge.Cuts-Linien, Bohrungen, Platinenmaß).

**Vorschau (JPG):** [`../fabrication/esp32-flrig-shield-preview.jpg`](../fabrication/esp32-flrig-shield-preview.jpg) — alle [Gerber](https://en.wikipedia.org/wiki/Gerber_format)-Layer (Kupfer, Mask, Paste, Silk, Bohrungen).

```bash
python3 render_pcb_preview.py --copy-docs   # JPG + SVG, optional in docs/assets/
```

Mit **[KiCad](https://de.wikipedia.org/wiki/KiCad) 7+** (`kicad-cli`) falls installiert, sonst automatisch **[Python](https://de.wikipedia.org/wiki/Python_(Programmiersprache))** (`kiutils` + `gerbonara` aus Projekt-`.venv`).

Alternativ in [KiCad](https://de.wikipedia.org/wiki/KiCad) GUI: **Datei → Plot** → [Gerber](https://en.wikipedia.org/wiki/Gerber_format) + Excellon → ZIP.

## 2. [JLCPCB](https://jlcpcb.com/) Parameter (empfohlen)

| Parameter | Wert |
|-----------|------|
| Layers | 2 |
| Material | FR-4 |
| Thickness | 1,6 mm |
| Copper | 1 oz |
| Surface | HASL (leadfree) oder ENIG |
| Min track/spacing | ≥ 6 mil (Design: 8 mil) |
| [PCB](https://de.wikipedia.org/wiki/Leiterplatte) Color | Schwarz / Grün nach Geschmack |

## 3. [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) Assembly

- **Top side only** (Komponenten laut [CPL](https://en.wikipedia.org/wiki/Pick-and-place)).
- **Bauteile ohne LCSC** in JLC-Editor als „Do not assemble“ markieren: U3, U4, J2 (Module/Header).
- `fabrication/BOM_JLCPCB.csv` hochladen, Footprints mit LCSC-Nummern abgleichen.
- **DNP (Do Not Place)** für optionale [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-C-Bestückung falls nicht in [BOM](https://de.wikipedia.org/wiki/St%C3%BCckliste).

### LCSC Referenzen (Prüfstand 2025 — vor Bestellung verifizieren)

| Ref | Teil | LCSC (Beispiel) |
|-----|------|-----------------|
| U1 | TXS0102DCUR | C15029 |
| U2 | [AMS1117](https://en.wikipedia.org/wiki/List_of_linear_integrated_circuits)-3.3 | C6186 |
| F1 | 500mA PTC | C13685 |
| D1 | SS34 | C8678 |
| C* | 100nF 0805 | C496 |
| R* | 10k/1k 0805 | C25744 / C11702 |
| J1 | [RJ45](https://de.wikipedia.org/wiki/RJ-45) 8P8C | C124375 / C27168 |

## 4. Checkliste vor „Add to Cart“

- [ ] [Gerber](https://en.wikipedia.org/wiki/Gerber_format) in JLC Viewer geöffnet — [RJ45](https://de.wikipedia.org/wiki/RJ-45) und J2 Lochbilder korrekt
- [ ] [BOM](https://de.wikipedia.org/wiki/St%C3%BCckliste)-Menge = [CPL](https://en.wikipedia.org/wiki/Pick-and-place)-Menge
- [ ] Shields mit **Stiftleiste** selbst löten (J2)
- [ ] [I2S](https://de.wikipedia.org/wiki/I%C2%B2S)-Module **nach** [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) handlöten
- [ ] Firmware `include/config.h` GPIOs unverändert zu J2-Tabelle

## 5. Kosten-Richtwert

~5–15 € für 5 PCBs (ohne Versand); +3–8 € [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) pro Board bei nur Basic Parts.

## 6. Nach Lieferung

→ [ASSEMBLY.md](ASSEMBLY.md) und [RJ45_PINOUT.md](RJ45_PINOUT.md)
