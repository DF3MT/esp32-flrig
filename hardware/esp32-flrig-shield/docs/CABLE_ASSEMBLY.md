# Bauanleitung — [RJ45](https://de.wikipedia.org/wiki/RJ-45)-Y-Kabel zum [Funkgerät](https://de.wikipedia.org/wiki/Transceiver)

## Benötigt

| Teil | Menge |
|------|-------|
| [RJ45](https://de.wikipedia.org/wiki/RJ-45)-Stecker (8P8C) | 1–2 |
| Cat-5e/6-Kabel, abgeschirmt | 1–3 m |
| 3,5 mm Mono- oder Stereo-Klinken | 2–3 |
| [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Adapterkabel (Gerät-spezifisch) | 1 |
| Schrumpfschlauch, Etiketten | — |

## Verdrahtung Funk-Ende (T568B-Farben → Funktion)

```
Pin 1 (ow)  ──► +5V   (nur wenn Funk 5V liefert — Strombedarf prüfen!)
Pin 2 (o)   ──► GND
Pin 3 (gw)  ──► Funk CAT TX (Daten vom Funk)
Pin 4 (g)   ──► Funk CAT RX (Daten zum Funk)
Pin 5 (bw)  ──► Funk LINE OUT / DATA OUT (+)
Pin 6 (b)   ──► Funk LINE IN / DATA IN (+)
Pin 7 (bw)  ──► GND (Audio-Masse, mit Pin 2 verbinden)
Pin 8 (br)  ──► offen (Reserve)
```

## Geräte-Hinweise

| Gerät | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | Audio RX | Audio TX |
|-------|-----|----------|----------|
| [ICOM](https://de.wikipedia.org/wiki/Icom) IC-7300 | REMOTE 3,5 mm | ACC VARI OUT | ACC MODU IN |
| [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-991A | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) Mini-DIN | [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) oder 1394 LINE OUT | LINE IN / [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) |
| [Xiegu](https://www.xiegu.com/) G90 | Display-IO [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | DATA OUT | [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)/Mic |
| [Xiegu](https://www.xiegu.com/) [X6100](https://www.xiegu.com/)/[X6200](https://www.xiegu.com/) | [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) Serial-B | [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) Audio | [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) Audio |

Bei **[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-only** Audio: [RJ45](https://de.wikipedia.org/wiki/RJ-45)-Pins 5/6 an **Line-Taps** eines [USB-Audio](https://de.wikipedia.org/wiki/Universal_Serial_Bus-Audioklasse)-Adapters oder an die **Interface-Platine** nur [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) (Pins 3/4) nutzen und Audio weiter über [WiFi](https://de.wikipedia.org/wiki/WLAN) (`ft8_remote.py`).

## Prüfung vor Inbetriebnahme

1. **Durchgang:** Pin 1↔1, 2↔2, … mit [Multimeter](https://de.wikipedia.org/wiki/Multimeter) (Kabel nicht am [PCB](https://de.wikipedia.org/wiki/Leiterplatte)).
2. **Kurzschluss:** 5V gegen GND ohne Last > 0,5 Ω → Fehler suchen.
3. **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver):** Baudrate im Funkmenü = Profil in ESP-Web-UI.
4. **Audio:** Ohne Funk, leises Rauschen auf AF_IN/AF_OUT normal.
