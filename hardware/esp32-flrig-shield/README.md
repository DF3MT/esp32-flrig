# ESP32-FLRIG Interface Shield (PCB Rev D, 78×50 mm)

<p align="center">
  <a href="#deutsch">🇩🇪 Deutsch</a> ·
  <a href="#english">🇬🇧 English</a>
</p>

<p align="center">
  <sub><strong><a href="https://en.wikipedia.org/wiki/Computer_Aided_Transceiver">CAT</a></strong> = <a href="https://en.wikipedia.org/wiki/Computer_Aided_Transceiver">Computer Aided Transceiver</a></sub>
</p>

![Blockdiagramm](assets/block_diagram.svg)

![PCB preview (Gerber)](fabrication/esp32-flrig-shield-preview.jpg)

---

<a id="deutsch"></a>

## Deutsch

### Übersicht

**78×50 mm**, **2-layer** — Shield für **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** ([USB-Host](https://de.wikipedia.org/wiki/USB)) oder optional **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (nur Funk **Kanal A**). Stecker an der **Vorderkante** (USB + RJ45), **CYD links**, **S3/Rotor hinten**: [PCB_LAYOUT.md](docs/PCB_LAYOUT.md).

| Anschluss | Funktion |
|-----------|----------|
| **J1 [RJ45](https://de.wikipedia.org/wiki/RJ-45)** | Funk **A**: +5 V, **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** ([UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle)), **Line-Audio** |
| **J5 / J6** | **[USB-A](https://de.wikipedia.org/wiki/USB) Buchse weiblich** — Funk **B** und **C** ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + Audio per [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)) |
| **U5 [USB2422](https://www.microchip.com/en-us/product/USB2422)** | 2-Port-Hub, Upstream **J3** → [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) OTG |
| **J7 / J3** | **[Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))** (2× Taster, 2× [Open-Collector](https://de.wikipedia.org/wiki/Offener_Kollektor)), parallel zu allen Funkkanälen |
| **J2** | 2×10 für **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (Kanal A + [I2S](https://de.wikipedia.org/wiki/I%C2%B2S), kein [USB-Host](https://de.wikipedia.org/wiki/USB)) |

- **U1 [ISO7741](https://www.ti.com/product/ISO7741)** — [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) [galvanisch getrennt](https://de.wikipedia.org/wiki/Galvanische_Trennung) ([RJ45](https://de.wikipedia.org/wiki/RJ-45))  
- **U6/U7 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** — [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) [galvanisch](https://de.wikipedia.org/wiki/Galvanische_Trennung) getrennt (Funk B/C)  
- **T1/T2** — [Audio-Trafos](https://de.wikipedia.org/wiki/Transformator) (Funk A)  
- **[PCM5102](https://www.ti.com/product/PCM5102) + [INMP441](https://www.invensense.com/products/digital/inmp441/)** — [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) auf ESP-Seite (48 [kHz](https://de.wikipedia.org/wiki/Hertz))  

**3× Funk + 1× [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))** gleichzeitig über **[WiFi](https://de.wikipedia.org/wiki/WLAN)**. **Keine gemeinsame Masse** zwischen den [Funkgeräten](https://de.wikipedia.org/wiki/Transceiver): [Isolation](../../docs/ISOLATION.md).

**Am PC:** drei **getrennte** [rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html)-Verbindungen (`:4532`, `:4536`, `:4540`) und drei **getrennte** Audio-[UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol)-Paare — kein Mix auf dem ESP; je Programm ein eigenes virtuelles Audiogerät (z. B. [VB-Cable](https://vb-audio.com/Cable/) A/B/C). Siehe [MULTI_RADIO_DE.md](../../docs/MULTI_RADIO_DE.md).

### Schnellstart

| Schritt | Dokument |
|---------|----------|
| 1. [Gerber](https://en.wikipedia.org/wiki/Gerber_format) bei [JLCPCB](https://jlcpcb.com/) | [JLCPCB_ORDER.md](docs/JLCPCB_ORDER.md) |
| 2. Platine löten | [ASSEMBLY.md](docs/ASSEMBLY.md) |
| 3. Kabel zum Funk | [CABLE_ASSEMBLY.md](docs/CABLE_ASSEMBLY.md) |
| 4. Schaltplan / Isolation | [SCHEMATIC.md](docs/SCHEMATIC.md) · [ISOLATION.md](../../docs/ISOLATION.md) |

### Fertige [Gerber](https://en.wikipedia.org/wiki/Gerber_format) & Vorschau

| Artefakt | Datei |
|----------|--------|
| [JLCPCB](https://jlcpcb.com/)-ZIP | [`fabrication/esp32-flrig-shield-jlc.zip`](fabrication/esp32-flrig-shield-jlc.zip) |
| JPG-Vorschau (alle Layer) | [`fabrication/esp32-flrig-shield-preview.jpg`](fabrication/esp32-flrig-shield-preview.jpg) |

```bash
cd kicad
./export_gerbers.sh
# oder:
python3 generate_gerbers.py --zip --preview
python3 render_pcb_preview.py --width 3600
```

> **Vor Produktion:** [DRC](https://en.wikipedia.org/wiki/Design_rule_check) in [KiCad](https://de.wikipedia.org/wiki/KiCad), Isolator-Footprints prüfen, **≥2 A** +5 V bei zwei [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-[Funkgeräten](https://de.wikipedia.org/wiki/Transceiver).

### Firmware

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

Mehr [Funkgeräte](https://de.wikipedia.org/wiki/Transceiver) / Ports: [MULTI_RADIO_DE.md](../../docs/MULTI_RADIO_DE.md) · Projekt: [Haupt-README](../../README.md)

### [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)) ([GPIO](https://de.wikipedia.org/wiki/General_Purpose_Input/Output))

| J7 / J3 | [GPIO](https://de.wikipedia.org/wiki/General_Purpose_Input/Output) (S3-Default) |
|---------|-------------------|
| Taster CCW | 27 |
| Taster CW | 5 |
| [Open Collector](https://de.wikipedia.org/wiki/Offener_Kollektor) CCW | 18 |
| [Open Collector](https://de.wikipedia.org/wiki/Offener_Kollektor) CW | 19 |

Am **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)**: J7 per Draht zu denselben GPIOs (siehe Haupt-README). **[rotctld](https://hamlib.sourceforge.net/html/rotctld.1.html)** Port **4535**.

### Dateien

```
esp32-flrig-shield/
├── kicad/              KiCad 7, generate_gerbers.py, render_pcb_preview.py
├── fabrication/        BOM, CPL, Gerber-ZIP, preview.jpg
├── docs/               Bauanleitung, RJ45, JLCPCB
└── assets/             Blockdiagramm (SVG)
```

---

<a id="english"></a>

## English

### Overview

**78×50 mm**, **2-layer** — shield for **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** ([USB host](https://en.wikipedia.org/wiki/USB)) or optionally **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (radio **channel A** only). Connectors on the **front edge** (USB + RJ45), **CYD on the left**, **S3/rotor on the back**: [PCB_LAYOUT.md](docs/PCB_LAYOUT.md).

| Connector | Function |
|-----------|----------|
| **J1 [RJ45](https://en.wikipedia.org/wiki/Registered_jack)** | Radio **A**: +5 V, **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** ([UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)), line audio |
| **J5 / J6** | **[USB Type-A](https://en.wikipedia.org/wiki/USB) receptacle** — radios **B** and **C** ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + audio via [USB](https://en.wikipedia.org/wiki/USB)) |
| **U5 [USB2422](https://www.microchip.com/en-us/product/USB2422)** | 2-port hub, upstream **J3** → [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) OTG |
| **J7 / J3** | **[Rotor](https://en.wikipedia.org/wiki/Antenna_rotator)** (2× buttons, 2× [open-collector](https://en.wikipedia.org/wiki/Open_collector)), in parallel with all radios |
| **J2** | 2×10 header for **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (channel A + [I2S](https://en.wikipedia.org/wiki/I%C2%B2S), no [USB host](https://en.wikipedia.org/wiki/USB)) |

- **U1 [ISO7741](https://www.ti.com/product/ISO7741)** — [galvanically isolated](https://en.wikipedia.org/wiki/Galvanic_isolation) [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([RJ45](https://en.wikipedia.org/wiki/Registered_jack))  
- **U6/U7 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** — [galvanically isolated](https://en.wikipedia.org/wiki/Galvanic_isolation) [USB](https://en.wikipedia.org/wiki/USB) (radios B/C)  
- **T1/T2** — [audio transformers](https://en.wikipedia.org/wiki/Transformer) (radio A)  
- **[PCM5102](https://www.ti.com/product/PCM5102) + [INMP441](https://www.invensense.com/products/digital/inmp441/)** — [I2S](https://en.wikipedia.org/wiki/I%C2%B2S) on ESP side (48 kHz)  

**3× radios + 1× [rotor](https://en.wikipedia.org/wiki/Antenna_rotator)** at once over **[WiFi](https://en.wikipedia.org/wiki/Wi-Fi)**. **No shared ground** between radios: [ISOLATION](../../docs/ISOLATION.md).

**On the PC:** three **separate** [rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html) connections (`:4532`, `:4536`, `:4540`) and three **separate** audio [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) port pairs — no mixing on the ESP; use one virtual audio device per app (e.g. [VB-Cable](https://vb-audio.com/Cable/) A/B/C). See [MULTI_RADIO_EN.md](../../docs/MULTI_RADIO_EN.md).

### Quick start

| Step | Document |
|------|----------|
| 1. Order at [JLCPCB](https://jlcpcb.com/) | [JLCPCB_ORDER.md](docs/JLCPCB_ORDER.md) |
| 2. Solder the board | [ASSEMBLY.md](docs/ASSEMBLY.md) |
| 3. Cables to the radio | [CABLE_ASSEMBLY.md](docs/CABLE_ASSEMBLY.md) |
| 4. Schematic / isolation | [SCHEMATIC.md](docs/SCHEMATIC.md) · [ISOLATION.md](../../docs/ISOLATION.md) |

### Gerbers & preview

| Artifact | File |
|----------|------|
| [JLCPCB](https://jlcpcb.com/) ZIP | [`fabrication/esp32-flrig-shield-jlc.zip`](fabrication/esp32-flrig-shield-jlc.zip) |
| JPG preview (all layers) | [`fabrication/esp32-flrig-shield-preview.jpg`](fabrication/esp32-flrig-shield-preview.jpg) |

```bash
cd kicad
./export_gerbers.sh
# or:
python3 generate_gerbers.py --zip --preview
python3 render_pcb_preview.py --width 3600
```

> **Before ordering:** run [DRC](https://en.wikipedia.org/wiki/Design_rule_check) in [KiCad](https://en.wikipedia.org/wiki/KiCad), verify isolator footprints, **≥2 A** at +5 V for two [USB](https://en.wikipedia.org/wiki/USB) radios.

### Firmware

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

Multi-radio / ports: [MULTI_RADIO_EN.md](../../docs/MULTI_RADIO_EN.md) · Project: [main README](../../README.md)

### [Rotor](https://en.wikipedia.org/wiki/Antenna_rotator) ([GPIO](https://en.wikipedia.org/wiki/General-purpose_input/output))

| J7 / J3 | [GPIO](https://en.wikipedia.org/wiki/General-purpose_input/output) (S3 default) |
|---------|-------------------|
| Button CCW | 27 |
| Button CW | 5 |
| [Open collector](https://en.wikipedia.org/wiki/Open_collector) CCW | 18 |
| [Open collector](https://en.wikipedia.org/wiki/Open_collector) CW | 19 |

On **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)**: wire **J7** to the same GPIOs (see main README). **[rotctld](https://hamlib.sourceforge.net/html/rotctld.1.html)** port **4535**.

### Files

```
esp32-flrig-shield/
├── kicad/              KiCad 7, generate_gerbers.py, render_pcb_preview.py
├── fabrication/        BOM, CPL, Gerber ZIP, preview.jpg
├── docs/               Assembly, RJ45, JLCPCB
└── assets/             Block diagram (SVG)
```

---

<p align="center">
  <sub><a href="#deutsch">🇩🇪</a> · <a href="#english">🇬🇧</a> · <a href="../../README.md">Project README</a></sub>
</p>
