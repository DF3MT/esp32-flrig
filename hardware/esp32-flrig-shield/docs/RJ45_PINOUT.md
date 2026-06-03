# [RJ45](https://de.wikipedia.org/wiki/RJ-45) Pinbelegung — [ESP32](https://de.wikipedia.org/wiki/ESP32)-FLRIG-1

Ein **8-poliges Cat-5e/Cat-6-Kabel** (oder fertiges Patchkabel mit abisolierten Adern am Funk-Ende) verbindet die **Interface-Platine** am [ESP32](https://de.wikipedia.org/wiki/ESP32) mit einer **Aufschaltbox am [Funkgerät](https://de.wikipedia.org/wiki/Transceiver)** ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + Audio + optional Strom).

## Steckerbelegung (Buchse auf [PCB](https://de.wikipedia.org/wiki/Leiterplatte), Blick auf Kontakte, Kabel abgehend)

| Pin | Farbe (T568B) | Signal | Richtung | Beschreibung |
|-----|---------------|--------|----------|--------------|
| 1 | orange/weiß | **+5V** | → Platine | Versorgung (max. **500 mA**), [Polyfuse](https://de.wikipedia.org/wiki/Polymer-Schmelzsicherung) auf [PCB](https://de.wikipedia.org/wiki/Leiterplatte) |
| 2 | orange | **GND** | — | Masse Hauptstrom |
| 3 | grün/weiß | **CAT_TX_RADIO** | Radio → ESP | [TTL](https://de.wikipedia.org/wiki/Transistor-Transistor-Logik) vom Funk (5 V), über [Level-Shifter](https://de.wikipedia.org/wiki/Pegelwandler) zu GPIO16 RX |
| 4 | grün | **CAT_RX_RADIO** | ESP → Radio | [TTL](https://de.wikipedia.org/wiki/Transistor-Transistor-Logik) zum Funk (5 V), von GPIO17 TX über Shifter |
| 5 | blau/weiß | **AF_IN** | Radio → Platine | Empfangs-Audio (Line-Out Funk), AC-gekoppelt |
| 6 | blau | **AF_OUT** | Platine → Radio | Sende-Audio (Line-In / Data-In Funk), AC-gekoppelt |
| 7 | braun/weiß | **GND_AUDIO** | — | Audio-Referenz (mit Pin 2 auf Funkseite verbinden) |
| 8 | braun | **NC** | — | Reserve (z. B. später [PTT](https://de.wikipedia.org/wiki/Sende-Empfang-Umschaltung) oder +5V-Sense) |

> **Wichtig:** Das ist **kein** Ethernet-Port. Kein Netzwerk-Switch anschließen.

## Signalübersicht

```
                    RJ45-Kabel (ESP32-FLRIG-1)
  ┌─────────────┐                              ┌──────────────────┐
  │  Shield PCB │  Pin1 +5V ──────────────────│ 5V (ACC/USB/Netzteil)│
  │  + CYD      │  Pin2 GND ──────────────────│ GND                │
  │             │  Pin3 ◄── CAT_TX (Funk)     │ CAT Buchse RX/TX   │
  │             │  Pin4 ──► CAT_RX (Funk)     │                    │
  │             │  Pin5 ◄── Line OUT Funk     │ 3,5 mm Line Out    │
  │             │  Pin6 ──► Line IN Funk      │ 3,5 mm Line/Data In│
  └─────────────┘  Pin7 GND Audio ────────────└──────────────────┘
```

## [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) [TTL](https://de.wikipedia.org/wiki/Transistor-Transistor-Logik)

- **Kreuzverdrahtung** auf der Funk-Seite: Pin 3 = Funk-TX → ESP empfängt; Pin 4 = Funk-RX ← ESP sendet.
- Pegel: **5 V [TTL](https://de.wikipedia.org/wiki/Transistor-Transistor-Logik)** am Funk; auf der Platine **[TXS0102](https://www.ti.com/product/TXS0102)** (3,3 V ↔ 5 V).

## Audio

- Pegel: **Line-Level** (~0,1–1,5 Vpp), nicht Lautsprecher-Taps.
- Auf der Platine: Trennkondensatoren 10 µF, Spannungsteiler zum [I2S](https://de.wikipedia.org/wiki/I%C2%B2S)-ADC; DAC-Ausgang über 10 µF + optional 1 kΩ Reihenwiderstand.
- **Mono** (nur Pin 5/6); Stereo-[Funkgeräte](https://de.wikipedia.org/wiki/Transceiver): L-Kanal verwenden, R über 7 auf GND.

## Stromversorgung

| Variante | Hinweis |
|----------|---------|
| **A** | Pin 1/2 von **5 V am Funk** (ACC, [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Port mit 500 mA) |
| **B** | Separates **5 V Netzteil** nur am [RJ45](https://de.wikipedia.org/wiki/RJ-45) (Funkseite Pins 3–8 trotzdem verbunden) |
| **C** | Zusätzlich **[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-C am [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)** nur zum Flashen (nicht gleichzeitig mit externem 5 V auf Pin 1 ohne Diode) |

Empfehlung auf [PCB](https://de.wikipedia.org/wiki/Leiterplatte): **[Schottky](https://de.wikipedia.org/wiki/Schottky-Diode) D1** ([USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) 5 V OR extern 5 V) — siehe Schaltplan.

## Empfohlenes Kabel

- **Am Panel:** abgeschirmtes [RJ45](https://de.wikipedia.org/wiki/RJ-45)-Patchkabel (≤ 3 m).
- **Am Funk:** eigene **Verteilerdose** oder **Gehäuse** mit:
  - 1× [RJ45](https://de.wikipedia.org/wiki/RJ-45) Buchse
  - 1× [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Stecker (3,5 mm TRS oder 6P6C je nach Gerät)
  - 2× 3,5 mm mono oder 1× stereo für Audio
  - Beschriftung nach Tabelle oben

Siehe [CABLE_ASSEMBLY.md](CABLE_ASSEMBLY.md) für Schritt-für-Schritt Lötanleitung der Funk-Seite.
