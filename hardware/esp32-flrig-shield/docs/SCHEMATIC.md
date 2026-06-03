# Schaltplan — ESP32-FLRIG Interface Shield (Rev A)

Logischer Schaltplan (KiCad-Quelle: [`../kicad/esp32-flrig-shield.kicad_sch`](../kicad/esp32-flrig-shield.kicad_sch)).

![Blockdiagramm](../assets/block_diagram.svg)

## Stromversorgung

```
RJ45 Pin1 (+5V) ── F1 (500mA) ── +5V_BUS ──┬── VCCB (TXS0102)
                                           ├── C11 10µF ── GND
                                           └── D1 (SS34) ──┬── USB_5V (optional CYD)
                                                           └── U2 AMS1117 IN
U2 OUT (+3V3) ── C12,C* ── +3V3 ── VCCA (TXS0102), U3, U4, J2 Pin2
RJ45 Pin2,7 ── GND
```

## CAT (Level-Shifter TXS0102)

```
J2 GPIO17 (ESP TX) ── A1 ── B1 ── RJ45 Pin4 (CAT_RX_RADIO → Funk)
J2 GPIO16 (ESP RX) ── A2 ── B2 ── RJ45 Pin3 (CAT_TX_RADIO ← Funk)
VCCA = 3V3, VCCB = +5V_BUS, OE = 3V3 (immer aktiv)
```

## I2S Audio (gemeinsamer Bus)

```
         +3V3
          │
    ┌─────┴─────┐
    │  PCM5102  │ BCK ◄── GPIO26 (J2-7) ──┐
    │   (U3)    │ LRCK ◄── GPIO25 (J2-8) ─┼── INMP441 (U4)
    │           │ DIN ◄── GPIO22 (J2-9)   │    SCK, WS gleich
    │  VOUT ────┼── C14 ── R3 ── RJ45 Pin6 (AF_OUT)
    └───────────┘
    ┌───────────┐
    │ INMP441   │ SD ──► GPIO4 (J2-10)
    │   (U4)    │
    └─────┬─────┘
RJ45 Pin5 (AF_IN) ── C13 ── R1/R2 (10k÷2) ── SD
```

## RJ45 — Netzliste

| RJ45 | Netz |
|------|------|
| 1 | +5V_IN |
| 2 | GND |
| 3 | CAT_TX_RADIO |
| 4 | CAT_RX_RADIO |
| 5 | AF_IN |
| 6 | AF_OUT |
| 7 | GND |
| 8 | NC |

## Spannungsteiler AF_IN (Line → INMP441)

Ziel: Line-Pegel ~1 Vpp auf INMP441-Eingang begrenzen.

```
AF_IN ── C13 10µF ──┬── R1 10k ── SD (U4)
                    └── R2 10k ── GND
```

## BOM-Kurzform

Siehe [ASSEMBLY.md](ASSEMBLY.md) und `fabrication/BOM_JLCPCB.csv`.
