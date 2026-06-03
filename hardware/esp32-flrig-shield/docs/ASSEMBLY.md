# Bauanleitung — [ESP32](https://de.wikipedia.org/wiki/ESP32)-FLRIG Interface Shield

Platine **REV C** (100×72 mm, **[galvanisch getrennt](https://de.wikipedia.org/wiki/Galvanische_Trennung)**) für [JLCPCB](https://jlcpcb.com). Firmware: `pio run -e esp32-flrig-shield` mit **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** an **J3**. [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) nur Kanal A (J2), ohne [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Isolation.

## Lieferumfang (selbst bestücken)

| Referenz | Bauteil | Montage | JLC [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) |
|----------|---------|---------|---------|
| U1 | [ISO7741](https://www.ti.com/product/ISO7741) ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Isolator, [SOIC](https://de.wikipedia.org/wiki/Small-outline_integrated_circuit)-16) | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| U6,U7 | [ADuM4160](https://www.analog.com/en/products/adum4160.html) ([USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Isolator je Port) | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| U8,U9 | [B0505S](https://en.wikipedia.org/wiki/DC-to-DC_converter) isoliertes DC/DC | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| T1,T2 | [Audio-Trafo](https://de.wikipedia.org/wiki/Transformator) 600:600 | [THT](https://de.wikipedia.org/wiki/Lochleiterplatte) | Hand |
| U2 | [AMS1117](https://en.wikipedia.org/wiki/List_of_linear_integrated_circuits)-3.3 SOT-223 | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| U3 | [PCM5102A](https://www.ti.com/product/PCM5102) Modul 6-Pin | **[THT](https://de.wikipedia.org/wiki/Lochleiterplatte) Hand** | Nein |
| U4 | [INMP441](https://www.invensense.com/products/digital/inmp441/) Modul 6-Pin | **[THT](https://de.wikipedia.org/wiki/Lochleiterplatte) Hand** | Nein |
| J1 | [RJ45](https://de.wikipedia.org/wiki/RJ-45) Buchse 8P8C mit Metallgehäuse | [THT](https://de.wikipedia.org/wiki/Lochleiterplatte) | Optional |
| U5 | [USB2422](https://www.microchip.com/en-us/product/USB2422) ([SSOP](https://de.wikipedia.org/wiki/Small-outline_integrated_circuit)-28) Hub | SMT | Ja |
| J5,J6 | **[USB-A](https://de.wikipedia.org/wiki/Universal_Serial_Bus) Buchse weiblich** (Host, zum Funk) | [THT](https://de.wikipedia.org/wiki/Lochleiterplatte) | Hand |
| J2 | Stiftleiste 2×10 [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) | [THT](https://de.wikipedia.org/wiki/Lochleiterplatte) | Nein |
| J3 | Stiftleiste 1×8 → [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) OTG + [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)) | THT | Nein |
| J7 | Stiftleiste 1×6 [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)) (parallel J3 Pin5–8) | [THT](https://de.wikipedia.org/wiki/Lochleiterplatte) | Nein |
| F2,F3 | [Polyfuse](https://de.wikipedia.org/wiki/Polymer-Schmelzsicherung) 500 mA je [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Port | SMT | Ja |
| F1 | [Polyfuse](https://de.wikipedia.org/wiki/Polymer-Schmelzsicherung) 500 mA | [THT](https://de.wikipedia.org/wiki/Lochleiterplatte)/[SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| D1 | SS34 [Schottky](https://de.wikipedia.org/wiki/Schottky-Diode) ([USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) OR Ext) | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| C1–C8 | 100 nF [Keramik](https://de.wikipedia.org/wiki/Keramikkondensator) 0805 | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| C9,C10 | 10 µF 0805 | SMT | Ja |
| C11,C12 | 10 µF [Elko](https://de.wikipedia.org/wiki/Elektrolytkondensator) oder 1206 | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| C13,C14 | 10 µF Film/Tantal Audio | THT | Nein |
| R1,R2 | 10 kΩ 0805 (AF_IN Teiler) | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| R3 | 1 kΩ 0805 (AF_OUT Serie) | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |
| R4,R5 | 10 kΩ 0805 ([I2S](https://de.wikipedia.org/wiki/I%C2%B2S) Pull-down) | [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage) | Ja |

Vollständige [Stückliste](https://de.wikipedia.org/wiki/St%C3%BCckliste): [`../fabrication/BOM_JLCPCB.csv`](../fabrication/BOM_JLCPCB.csv)

## Werkzeuge

- [Lötkolben](https://de.wikipedia.org/wiki/L%C3%B6tkolben) 320–380 °C, feine Spitze, Flussmittel
- Lupe / Mikroskop für [SSOP](https://de.wikipedia.org/wiki/Small-outline_integrated_circuit)-8
- [Multimeter](https://de.wikipedia.org/wiki/Multimeter)
- [KiCad](https://de.wikipedia.org/wiki/KiCad) 7+ zum Prüfen von [Gerbern](https://en.wikipedia.org/wiki/Gerber_format) vor Bestellung

## Montagereihenfolge

### 1. [SMT](https://de.wikipedia.org/wiki/Oberfl%C3%A4chenmontage)-Bauteile (oder [JLCPCB](https://jlcpcb.com/) Assembly)

1. **U2** [AMS1117](https://en.wikipedia.org/wiki/List_of_linear_integrated_circuits) — mit Kühlfläche zur Masse, kurze 3,3-V-Leiterbahnen.
2. **U1** [TXS0102](https://www.ti.com/product/TXS0102) — Pin 1 Markierung beachten; **VCCA** an 3V3, **VCCB** an +5V_RJ.
3. Widerstände und **100 nF** — je nahe am IC-Versorgungspin.
4. **F1**, **D1** — Strompfad: [RJ45](https://de.wikipedia.org/wiki/RJ-45) Pin1 → F1 → +5V_BUS → D1 → ([USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) \| [RJ45](https://de.wikipedia.org/wiki/RJ-45)) → U2 → 3V3.
5. **C11/C12** an 5V und 3,3V Bussen.

### 2. [THT](https://de.wikipedia.org/wiki/Lochleiterplatte) — Leistung & Buchsen

1. **J1** [RJ45](https://de.wikipedia.org/wiki/RJ-45) — Gehäusefüße zuerst löten (Zugentlastung), dann Pins.
2. **J2** 2×10 Stiftleiste **auf Oberseite** ([CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) steckt von oben auf).

### 3. [I2S](https://de.wikipedia.org/wiki/I%C2%B2S)-Module (Hand)

**[PCM5102](https://www.ti.com/product/PCM5102) (U3)** — Beschriftung zum Schaltplan:

| Modul | [PCB](https://de.wikipedia.org/wiki/Leiterplatte) Net |
|-------|---------|
| VCC | +3V3 |
| GND | GND |
| BCK | I2S_BCLK |
| LCK / LRCK | I2S_LRCK |
| DIN | I2S_DOUT |
| SCK | GND (Modul-Anleitung) |

**[INMP441](https://www.invensense.com/products/digital/inmp441/) (U4)**:

| Modul | [PCB](https://de.wikipedia.org/wiki/Leiterplatte) Net |
|-------|---------|
| VDD | +3V3 |
| GND | GND |
| SCK | I2S_BCLK |
| WS | I2S_LRCK |
| SD | I2S_DIN |
| L/R | GND (Mono links) |

### 4. Audio AC-Kopplung ([RJ45](https://de.wikipedia.org/wiki/RJ-45))

- **C13** zwischen **J1 Pin5** und Knoten **AF_IN** (vor R1/R2 Teiler zum [INMP441](https://www.invensense.com/products/digital/inmp441/)-Pfad).
- **C14** zwischen **[PCM5102](https://www.ti.com/product/PCM5102) Ausgang** (VOUTL) und **J1 Pin6** über **R3**.

> Für **Line-In** statt MEMS: optional [INMP441](https://www.invensense.com/products/digital/inmp441/) weglassen und **PCM1808**-Footprint bestücken (Rev B) — Rev A nutzt [INMP441](https://www.invensense.com/products/digital/inmp441/) + Pegelabsenkung von [RJ45](https://de.wikipedia.org/wiki/RJ-45).

### 5. [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) aufstecken

| J2 Pin | [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) [GPIO](https://de.wikipedia.org/wiki/General_Purpose_Input/Output) / Net |
|--------|----------------|
| 1 | 5V (VIN) |
| 2 | 3V3 (nur Monitoring, optional NC) |
| 3,4 | GND |
| 5 | GPIO17 TX |
| 6 | GPIO16 RX |
| 7 | GPIO26 BCLK |
| 8 | GPIO25 LRCK |
| 9 | GPIO22 |
| 10 | GPIO4 |
| 11–20 | GND (empfohlen mehrfach) |

**Nicht gleichzeitig:** [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-C am [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) **und** +5V auf [RJ45](https://de.wikipedia.org/wiki/RJ-45) Pin1 ohne [Schottky](https://de.wikipedia.org/wiki/Schottky-Diode) OR — **D1** muss bestückt sein.

## Erste Inbetriebnahme

1. **Isolation:** Zwischen **GND_ESP** (J2 Pin 3/4) und **GND_RADIO_A** ([RJ45](https://de.wikipedia.org/wiki/RJ-45) Pin 2) → **∞ Ω** (kein Durchgang). Ebenso J5-GND vs. J6-GND vs. ESP.
2. **Ohne [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display):** 3,3V an J2 (≈ 3,3 V ± 5 %). [RJ45](https://de.wikipedia.org/wiki/RJ-45) +5V nur Funk-A-Domäne.
3. **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)):** Funk aus, [ISO7741](https://www.ti.com/product/ISO7741) Versorgung beidseitig, keine GND-Brücke.
3. **[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) flashen**, Web-UI: Funkprofil wählen, Baudrate prüfen.
4. **Audio:** `http://<ESP-IP>/audio` oder `scripts/ft8_remote.py <IP> --list-devices`.

## Fehlersuche

| Symptom | Prüfung |
|---------|---------|
| Kein 3,3 V | F1, U2, D1 Polarität, [RJ45](https://de.wikipedia.org/wiki/RJ-45) Pin1/2 |
| [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) tot | TX/RX vertauscht?, Baudrate, Shifter VCCB |
| Verzerrtes Audio | AF Pegel zu hoch → Teiler, kein Speaker-Out |
| Brummen | Pin7 mit Pin2 verbunden, kurze Masse |

## Revisionen

| Rev | Änderung |
|-----|----------|
| A | Erstlayout, [TXS0102](https://www.ti.com/product/TXS0102), [PCM5102](https://www.ti.com/product/PCM5102)+[INMP441](https://www.invensense.com/products/digital/inmp441/) Module, [RJ45](https://de.wikipedia.org/wiki/RJ-45) [ESP32](https://de.wikipedia.org/wiki/ESP32)-FLRIG-1 |
