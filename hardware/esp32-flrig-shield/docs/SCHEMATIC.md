# Schaltplan — [ESP32](https://de.wikipedia.org/wiki/ESP32)-FLRIG Interface Shield (Rev C)

Logischer Schaltplan ([KiCad](https://de.wikipedia.org/wiki/KiCad): [`../kicad/esp32-flrig-shield.kicad_pcb`](../kicad/esp32-flrig-shield.kicad_pcb)).

![Blockdiagramm](../assets/block_diagram.svg)

**Rev C** = Rev B + **galvanische Trennung** zwischen allen Funk-Massebereichen ([`../../../docs/ISOLATION.md`](../../../docs/ISOLATION.md)).

**[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** = **[Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** (Steuerprotokoll), nicht „Caterpillar“.

## Masse-Domänen (kein gemeinsamer GND-Kupfer!)

| Netz | Verbindung |
|------|------------|
| **GND_ESP** | [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)/[CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display), U5 Hub Upstream, U1/U6/U7 Primärseite, [I2S](https://de.wikipedia.org/wiki/I%C2%B2S)-Module ESP-Seite |
| **GND_RADIO_A** | [RJ45](https://de.wikipedia.org/wiki/RJ-45) Pin 2/7, U1 Sekundärseite, T1/T2 Funk-Seite |
| **GND_USB_B** | J5 Shield/GND, U6 Sekundärseite, U8 DC/DC Ausgang |
| **GND_USB_C** | J6 Shield/GND, U7 Sekundärseite, U9 DC/DC Ausgang |

Zwischen diesen Netzen nur **Isolatoren** (und optional HF-RC am [RJ45](https://de.wikipedia.org/wiki/RJ-45)-Shield).

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

## Funk A — [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver))

**U1 [ISO7741](https://www.ti.com/product/ISO7741)** (ersetzt [TXS0102](https://www.ti.com/product/TXS0102) — kein reiner [Pegelwandler](https://de.wikipedia.org/wiki/Pegelwandler) mehr):

```
GPIO16/17 (ESP) ── ISO7741 Seite A ──║── Seite B ── RJ45 Pin 3/4 (CAT TTL Funk)
+3V3_ESP              GND_ESP          GND_RADIO_A      +5V_RADIO (RJ45 Pin1, lokaler Bezug)
```

Optional **5 V ↔ 3,3 V** auf **Funk-Seite** nur für kompatible Pegel — **ohne** GND-Brücke zur ESP-Masse.

## Funk A — Audio ([galvanisch getrennt](https://de.wikipedia.org/wiki/Galvanische_Trennung))

```
PCM5102 / INMP441 (ESP-Seite, GND_ESP) ── T1/T2 1:1 Audio-Trafo ── RJ45 Pin 5/6
```

Kein direkter DC-Pfad AF_IN/AF_OUT zwischen [I2S](https://de.wikipedia.org/wiki/I%C2%B2S)-Modulen und [RJ45](https://de.wikipedia.org/wiki/RJ-45).

## [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) Funk B / C

```
ESP USB OTG ── U5 USB2422 (nur GND_ESP)
                 ├── Down1 ── U6 ADuM4160 ── J5 (USB-A)  GND_USB_B
                 └── Down2 ── U7 ADuM4160 ── J6 (USB-A)  GND_USB_C
U8/U9: isoliertes DC/DC (z. B. B0505S) +5V_BUS → VBUS je Port (Sekundär getrennt)
```

**Wichtig:** J5- und J6-**Masse nicht** mit [RJ45](https://de.wikipedia.org/wiki/RJ-45)-Masse oder ESP-Masse verbinden.

## Stromversorgung

- **+5V_BUS**: ESP-Shield / externes Netzteil (GND_REF = **GND_ESP** nur).
- **[RJ45](https://de.wikipedia.org/wiki/RJ-45) Pin 1**: bevorzugt nur **Funk-A-Sekundär** (Isolator, Trafos) — nicht als Rückleiter für [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-B/C.
- **≥2 A** empfohlen, wenn zwei isolierte [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Ports laden.

## [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))

J7 → J3 [GPIO](https://de.wikipedia.org/wiki/General_Purpose_Input/Output) (Taster + OC). [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))-Relais-Masse **extern** mit eigenem Netzteil; **nicht** mit Funk-B/C-Masse verbunden, wenn Brummen vermieden werden soll.

## [RJ45](https://de.wikipedia.org/wiki/RJ-45) — Netzliste (Funk A, isoliert)

| [RJ45](https://de.wikipedia.org/wiki/RJ-45) | Netz |
|------|------|
| 1 | +5V_RADIO_A |
| 2 | GND_RADIO_A |
| 3 | CAT_TX_RADIO (nach U1) |
| 4 | CAT_RX_RADIO (nach U1) |
| 5 | AF_IN (nach T1) |
| 6 | AF_OUT (nach T2) |
| 7 | GND_RADIO_A |
| 8 | NC |

## [BOM](https://de.wikipedia.org/wiki/St%C3%BCckliste) Rev C

Siehe `fabrication/BOM_JLCPCB.csv` — **U1 [ISO7741](https://www.ti.com/product/ISO7741)**, **U6/U7 [ADuM4160](https://www.analog.com/en/products/adum4160.html)**, **U8/U9 [B0505S](https://en.wikipedia.org/wiki/DC-to-DC_converter)**, **T1/T2 [Audio-Trafo](https://de.wikipedia.org/wiki/Transformator)**.
