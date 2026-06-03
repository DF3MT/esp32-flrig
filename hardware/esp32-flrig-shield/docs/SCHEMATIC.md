# Schaltplan — ESP32-FLRIG Interface Shield (Rev C)

Logischer Schaltplan (KiCad: [`../kicad/esp32-flrig-shield.kicad_pcb`](../kicad/esp32-flrig-shield.kicad_pcb)).

![Blockdiagramm](../assets/block_diagram.svg)

**Rev C** = Rev B + **galvanische Trennung** zwischen allen Funk-Massebereichen ([`../../../docs/ISOLATION.md`](../../../docs/ISOLATION.md)).

**CAT** = **Computer Aided Transceiver** (Steuerprotokoll), nicht „Caterpillar“.

## Masse-Domänen (kein gemeinsamer GND-Kupfer!)

| Netz | Verbindung |
|------|------------|
| **GND_ESP** | ESP32-S3/CYD, U5 Hub Upstream, U1/U6/U7 Primärseite, I2S-Module ESP-Seite |
| **GND_RADIO_A** | RJ45 Pin 2/7, U1 Sekundärseite, T1/T2 Funk-Seite |
| **GND_USB_B** | J5 Shield/GND, U6 Sekundärseite, U8 DC/DC Ausgang |
| **GND_USB_C** | J6 Shield/GND, U7 Sekundärseite, U9 DC/DC Ausgang |

Zwischen diesen Netzen nur **Isolatoren** (und optional HF-RC am RJ45-Shield).

## Gesamtübersicht

```
                    ┌── GND_ESP ──────────────────────────────┐
Funk A RJ45 ── U1 ISO7741 ── CAT UART    T1/T2 ── AF (nur über Trafos)
     GND_RADIO_A (isoliert)                │
Funk B J5 ──── U6 ADuM4160 ◄── U5 Hub Down1 ── ESP USB OTG
     GND_USB_B + U8 B0505S VBUS
Funk C J6 ──── U7 ADuM4160 ◄── U5 Hub Down2
     GND_USB_C + U9 B0505S VBUS
Rotor J7 ───── direkt GPIO (gemeinsame ESP-Masse, kein Funk-GND)
```

## Funk A — CAT (Computer Aided Transceiver)

**U1 ISO7741** (ersetzt TXS0102 — kein reiner Pegelwandler mehr):

```
GPIO16/17 (ESP) ── ISO7741 Seite A ──║── Seite B ── RJ45 Pin 3/4 (CAT TTL Funk)
+3V3_ESP              GND_ESP          GND_RADIO_A      +5V_RADIO (RJ45 Pin1, lokaler Bezug)
```

Optional **5 V ↔ 3,3 V** auf **Funk-Seite** nur für kompatible Pegel — **ohne** GND-Brücke zur ESP-Masse.

## Funk A — Audio (galvanisch getrennt)

```
PCM5102 / INMP441 (ESP-Seite, GND_ESP) ── T1/T2 1:1 Audio-Trafo ── RJ45 Pin 5/6
```

Kein direkter DC-Pfad AF_IN/AF_OUT zwischen I2S-Modulen und RJ45.

## USB Funk B / C

```
ESP USB OTG ── U5 USB2422 (nur GND_ESP)
                 ├── Down1 ── U6 ADuM4160 ── J5 (USB-A)  GND_USB_B
                 └── Down2 ── U7 ADuM4160 ── J6 (USB-A)  GND_USB_C
U8/U9: isoliertes DC/DC (z. B. B0505S) +5V_BUS → VBUS je Port (Sekundär getrennt)
```

**Wichtig:** J5- und J6-**Masse nicht** mit RJ45-Masse oder ESP-Masse verbinden.

## Stromversorgung

- **+5V_BUS**: ESP-Shield / externes Netzteil (GND_REF = **GND_ESP** nur).
- **RJ45 Pin 1**: bevorzugt nur **Funk-A-Sekundär** (Isolator, Trafos) — nicht als Rückleiter für USB-B/C.
- **≥2 A** empfohlen, wenn zwei isolierte USB-Ports laden.

## Rotor

J7 → J3 GPIO (Taster + OC). Rotor-Relais-Masse **extern** mit eigenem Netzteil; **nicht** mit Funk-B/C-Masse verbunden, wenn Brummen vermieden werden soll.

## RJ45 — Netzliste (Funk A, isoliert)

| RJ45 | Netz |
|------|------|
| 1 | +5V_RADIO_A |
| 2 | GND_RADIO_A |
| 3 | CAT_TX_RADIO (nach U1) |
| 4 | CAT_RX_RADIO (nach U1) |
| 5 | AF_IN (nach T1) |
| 6 | AF_OUT (nach T2) |
| 7 | GND_RADIO_A |
| 8 | NC |

## BOM Rev C

Siehe `fabrication/BOM_JLCPCB.csv` — **U1 ISO7741**, **U6/U7 ADuM4160**, **U8/U9 B0505S**, **T1/T2 Audio-Trafo**.
