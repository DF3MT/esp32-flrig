# ESP32-FLRIG Interface Shield

<p align="center">
  <a href="#deutsch">🇩🇪 Deutsch</a> ·
  <a href="#english">🇬🇧 English</a>
</p>

<p align="center">
  <sub><strong><a href="https://en.wikipedia.org/wiki/Computer_Aided_Transceiver">CAT</a></strong> = <a href="https://en.wikipedia.org/wiki/Computer_Aided_Transceiver">Computer Aided Transceiver</a></sub>
</p>

![Blockdiagramm](assets/block_diagram.svg)

---

<a id="deutsch"></a>

## Deutsch

### Übersicht

Dokumentation für ein **Interface-Shield** zwischen **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** ([USB-Host](https://de.wikipedia.org/wiki/USB)) bzw. optional **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (nur Funk **Kanal A**) und bis zu **drei Funkgeräten**:

| Anschluss | Funktion |
|-----------|----------|
| **J1 [RJ45](https://de.wikipedia.org/wiki/RJ-45)** | Funk **A**: +5 V, **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** ([UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle)), **Line-Audio** |
| **J5 / J6** | **[USB-A](https://de.wikipedia.org/wiki/USB) Buchse** — Funk **B** und **C** ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + Audio per [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)) |
| **U5 [USB2422](https://www.microchip.com/en-us/product/USB2422)** | 2-Port-Hub, Upstream **J3** → [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) OTG |
| **J7 / J3** | **[Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))** (2× Taster, 2× [Open-Collector](https://de.wikipedia.org/wiki/Offener_Kollektor)) |
| **J2** | 2×10 für **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (Kanal A + [I2S](https://de.wikipedia.org/wiki/I%C2%B2S)) |

- **U1 [ISO7741](https://www.ti.com/product/ISO7741)** — [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) [galvanisch getrennt](https://de.wikipedia.org/wiki/Galvanische_Trennung) ([RJ45](https://de.wikipedia.org/wiki/RJ-45))  
- **U6/U7 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** — [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) [galvanisch](https://de.wikipedia.org/wiki/Galvanische_Trennung) getrennt (Funk B/C)  
- **T1/T2** — [Audio-Trafos](https://de.wikipedia.org/wiki/Transformator) (Funk A)  
- **[PCM5102](https://www.ti.com/product/PCM5102) + [INMP441](https://www.invensense.com/products/digital/inmp441/)** — [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) (48 [kHz](https://de.wikipedia.org/wiki/Hertz))  

**3× Funk + 1× [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))** über **[WiFi](https://de.wikipedia.org/wiki/WLAN)**. **Keine gemeinsame Masse** zwischen den [Funkgeräten](https://de.wikipedia.org/wiki/Transceiver): [Isolation](../../docs/ISOLATION.md).

**Am PC:** drei getrennte [rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html)-Verbindungen und drei Audio-[UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol)-Paare — siehe [MULTI_RADIO_DE.md](../../docs/MULTI_RADIO_DE.md).

### Dokumentation

| Thema | Datei |
|-------|--------|
| Logischer Schaltplan | [SCHEMATIC.md](docs/SCHEMATIC.md) |
| Kabel / RJ45 | [CABLE_ASSEMBLY.md](docs/CABLE_ASSEMBLY.md) · [RJ45_PINOUT.md](docs/RJ45_PINOUT.md) |
| Isolation | [ISOLATION.md](../../docs/ISOLATION.md) |

### Firmware

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

### [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)) ([GPIO](https://de.wikipedia.org/wiki/General_Purpose_Input/Output))

| J7 / J3 | [GPIO](https://de.wikipedia.org/wiki/General_Purpose_Input/Output) (S3-Default) |
|---------|-------------------|
| Taster CCW | 27 |
| Taster CW | 5 |
| [Open Collector](https://de.wikipedia.org/wiki/Offener_Kollektor) CCW | 18 |
| [Open Collector](https://de.wikipedia.org/wiki/Offener_Kollektor) CW | 19 |

**[rotctld](https://hamlib.sourceforge.net/html/rotctld.1.html)** Port **4535**.

### Dateien

```
esp32-flrig-shield/
├── docs/     Schaltplan, Kabel, RJ45
└── assets/   Blockdiagramm (SVG)
```

---

<a id="english"></a>

## English

### Overview

Documentation for an **interface shield** between **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** ([USB host](https://en.wikipedia.org/wiki/USB)) or optionally **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** (radio **channel A** only) and up to **three radios**:

| Connector | Function |
|-----------|----------|
| **J1 [RJ45](https://en.wikipedia.org/wiki/Registered_jack)** | Radio **A**: +5 V, **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** ([UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)), line audio |
| **J5 / J6** | **[USB Type-A](https://en.wikipedia.org/wiki/USB)** — radios **B** and **C** |
| **U5 [USB2422](https://www.microchip.com/en-us/product/USB2422)** | 2-port hub, upstream **J3** → [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) OTG |
| **J7 / J3** | **[Rotor](https://en.wikipedia.org/wiki/Antenna_rotator)** (2× buttons, 2× [open-collector](https://en.wikipedia.org/wiki/Open_collector)) |
| **J2** | 2×10 header for **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** |

See [ISOLATION.md](../../docs/ISOLATION.md) and [MULTI_RADIO_EN.md](../../docs/MULTI_RADIO_EN.md).

### Documentation

| Topic | File |
|-------|------|
| Logical schematic | [SCHEMATIC.md](docs/SCHEMATIC.md) |
| Cables / RJ45 | [CABLE_ASSEMBLY.md](docs/CABLE_ASSEMBLY.md) · [RJ45_PINOUT.md](docs/RJ45_PINOUT.md) |

### Firmware

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

### [Rotor](https://en.wikipedia.org/wiki/Antenna_rotator) ([GPIO](https://en.wikipedia.org/wiki/General-purpose_input/output))

| J7 / J3 | GPIO (S3 default) |
|---------|-------------------|
| Button CCW | 27 |
| Button CW | 5 |
| [Open collector](https://en.wikipedia.org/wiki/Open_collector) CCW | 18 |
| [Open collector](https://en.wikipedia.org/wiki/Open_collector) CW | 19 |

**[rotctld](https://hamlib.sourceforge.net/html/rotctld.1.html)** port **4535**.

---

<p align="center">
  <sub><a href="#deutsch">🇩🇪</a> · <a href="#english">🇬🇧</a> · <a href="../../README.md">Project README</a></sub>
</p>
