# Bauanleitung — RJ45-Y-Kabel zum Funkgerät

## Benötigt

| Teil | Menge |
|------|-------|
| RJ45-Stecker (8P8C) | 1–2 |
| Cat-5e/6-Kabel, abgeschirmt | 1–3 m |
| 3,5 mm Mono- oder Stereo-Klinken | 2–3 |
| CAT-Adapterkabel (Gerät-spezifisch) | 1 |
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

| Gerät | CAT | Audio RX | Audio TX |
|-------|-----|----------|----------|
| ICOM IC-7300 | REMOTE 3,5 mm | ACC VARI OUT | ACC MODU IN |
| Yaesu FT-991A | CAT Mini-DIN | USB oder 1394 LINE OUT | LINE IN / USB |
| Xiegu G90 | Display-IO CAT | DATA OUT | USB/Mic |
| Xiegu X6100/X6200 | USB Serial-B | USB Audio | USB Audio |

Bei **USB-only** Audio: RJ45-Pins 5/6 an **Line-Taps** eines USB-Audio-Adapters oder an die **Interface-Platine** nur CAT (Pins 3/4) nutzen und Audio weiter über WiFi (`ft8_remote.py`).

## Prüfung vor Inbetriebnahme

1. **Durchgang:** Pin 1↔1, 2↔2, … mit Multimeter (Kabel nicht am PCB).
2. **Kurzschluss:** 5V gegen GND ohne Last > 0,5 Ω → Fehler suchen.
3. **CAT:** Baudrate im Funkmenü = Profil in ESP-Web-UI.
4. **Audio:** Ohne Funk, leises Rauschen auf AF_IN/AF_OUT normal.
