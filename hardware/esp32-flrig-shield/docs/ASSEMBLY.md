# Bauanleitung — ESP32-FLRIG Interface Shield

Platine **REV C** (100×72 mm, **galvanisch getrennt**) für [JLCPCB](https://jlcpcb.com). Firmware: `pio run -e esp32-flrig-shield` mit **ESP32-S3** an **J3**. CYD nur Kanal A (J2), ohne USB-Isolation.

## Lieferumfang (selbst bestücken)

| Referenz | Bauteil | Montage | JLC SMT |
|----------|---------|---------|---------|
| U1 | ISO7741 (CAT-Isolator, SOIC-16) | SMT | Ja |
| U6,U7 | ADuM4160 (USB-Isolator je Port) | SMT | Ja |
| U8,U9 | B0505S isoliertes DC/DC | SMT | Ja |
| T1,T2 | Audio-Trafo 600:600 | THT | Hand |
| U2 | AMS1117-3.3 SOT-223 | SMT | Ja |
| U3 | PCM5102A Modul 6-Pin | **THT Hand** | Nein |
| U4 | INMP441 Modul 6-Pin | **THT Hand** | Nein |
| J1 | RJ45 Buchse 8P8C mit Metallgehäuse | THT | Optional |
| U5 | USB2422 (SSOP-28) Hub | SMT | Ja |
| J5,J6 | **USB-A Buchse weiblich** (Host, zum Funk) | THT | Hand |
| J2 | Stiftleiste 2×10 CYD | THT | Nein |
| J3 | Stiftleiste 1×8 → ESP32-S3 OTG + Rotor | THT | Nein |
| J7 | Stiftleiste 1×6 Rotor (parallel J3 Pin5–8) | THT | Nein |
| F2,F3 | Polyfuse 500 mA je USB-Port | SMT | Ja |
| F1 | Polyfuse 500 mA | THT/SMT | Ja |
| D1 | SS34 Schottky (USB OR Ext) | SMT | Ja |
| C1–C8 | 100 nF Keramik 0805 | SMT | Ja |
| C9,C10 | 10 µF 0805 | SMT | Ja |
| C11,C12 | 10 µF Elko oder 1206 | SMT | Ja |
| C13,C14 | 10 µF Film/Tantal Audio | THT | Nein |
| R1,R2 | 10 kΩ 0805 (AF_IN Teiler) | SMT | Ja |
| R3 | 1 kΩ 0805 (AF_OUT Serie) | SMT | Ja |
| R4,R5 | 10 kΩ 0805 (I2S Pull-down) | SMT | Ja |

Vollständige Stückliste: [`../fabrication/BOM_JLCPCB.csv`](../fabrication/BOM_JLCPCB.csv)

## Werkzeuge

- Lötkolben 320–380 °C, feine Spitze, Flussmittel
- Lupe / Mikroskop für SSOP-8
- Multimeter
- KiCad 7+ zum Prüfen von Gerbern vor Bestellung

## Montagereihenfolge

### 1. SMT-Bauteile (oder JLCPCB Assembly)

1. **U2** AMS1117 — mit Kühlfläche zur Masse, kurze 3,3-V-Leiterbahnen.
2. **U1** TXS0102 — Pin 1 Markierung beachten; **VCCA** an 3V3, **VCCB** an +5V_RJ.
3. Widerstände und **100 nF** — je nahe am IC-Versorgungspin.
4. **F1**, **D1** — Strompfad: RJ45 Pin1 → F1 → +5V_BUS → D1 → (USB \| RJ45) → U2 → 3V3.
5. **C11/C12** an 5V und 3,3V Bussen.

### 2. THT — Leistung & Buchsen

1. **J1** RJ45 — Gehäusefüße zuerst löten (Zugentlastung), dann Pins.
2. **J2** 2×10 Stiftleiste **auf Oberseite** (CYD steckt von oben auf).

### 3. I2S-Module (Hand)

**PCM5102 (U3)** — Beschriftung zum Schaltplan:

| Modul | PCB Net |
|-------|---------|
| VCC | +3V3 |
| GND | GND |
| BCK | I2S_BCLK |
| LCK / LRCK | I2S_LRCK |
| DIN | I2S_DOUT |
| SCK | GND (Modul-Anleitung) |

**INMP441 (U4)**:

| Modul | PCB Net |
|-------|---------|
| VDD | +3V3 |
| GND | GND |
| SCK | I2S_BCLK |
| WS | I2S_LRCK |
| SD | I2S_DIN |
| L/R | GND (Mono links) |

### 4. Audio AC-Kopplung (RJ45)

- **C13** zwischen **J1 Pin5** und Knoten **AF_IN** (vor R1/R2 Teiler zum INMP441-Pfad).
- **C14** zwischen **PCM5102 Ausgang** (VOUTL) und **J1 Pin6** über **R3**.

> Für **Line-In** statt MEMS: optional INMP441 weglassen und **PCM1808**-Footprint bestücken (Rev B) — Rev A nutzt INMP441 + Pegelabsenkung von RJ45.

### 5. CYD aufstecken

| J2 Pin | CYD GPIO / Net |
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

**Nicht gleichzeitig:** USB-C am CYD **und** +5V auf RJ45 Pin1 ohne Schottky OR — **D1** muss bestückt sein.

## Erste Inbetriebnahme

1. **Isolation:** Zwischen **GND_ESP** (J2 Pin 3/4) und **GND_RADIO_A** (RJ45 Pin 2) → **∞ Ω** (kein Durchgang). Ebenso J5-GND vs. J6-GND vs. ESP.
2. **Ohne CYD:** 3,3V an J2 (≈ 3,3 V ± 5 %). RJ45 +5V nur Funk-A-Domäne.
3. **CAT (Computer Aided Transceiver):** Funk aus, ISO7741 Versorgung beidseitig, keine GND-Brücke.
3. **CYD flashen**, Web-UI: Funkprofil wählen, Baudrate prüfen.
4. **Audio:** `http://<ESP-IP>/audio` oder `scripts/ft8_remote.py <IP> --list-devices`.

## Fehlersuche

| Symptom | Prüfung |
|---------|---------|
| Kein 3,3 V | F1, U2, D1 Polarität, RJ45 Pin1/2 |
| CAT tot | TX/RX vertauscht?, Baudrate, Shifter VCCB |
| Verzerrtes Audio | AF Pegel zu hoch → Teiler, kein Speaker-Out |
| Brummen | Pin7 mit Pin2 verbunden, kurze Masse |

## Revisionen

| Rev | Änderung |
|-----|----------|
| A | Erstlayout, TXS0102, PCM5102+INMP441 Module, RJ45 ESP32-FLRIG-1 |
