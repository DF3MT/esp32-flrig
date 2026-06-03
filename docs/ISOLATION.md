# Galvanic isolation between transceivers

[Deutsch](#deutsch) · [English](#english)

Switching between radios must **not** create a shared, low-impedance ground path between rigs (ground loops, hum, CAT errors). Shield **Rev C** uses **separate ground domains** linked only through isolators.

<a id="deutsch"></a>

## Deutsch

### Problem

Ohne Trennung sind **[ESP32](https://de.wikipedia.org/wiki/ESP32)-Masse**, **Funk-A-Masse** ([RJ45](https://de.wikipedia.org/wiki/RJ-45)), **[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-B-Masse** und **[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-C-Masse** über Kabel und Shield verbunden → **Brummen**, falsche [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Bytes, [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Abbrüche beim Wechsel zwischen mehreren Geräten.

### Lösung (Rev C)

| Pfad | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)) | Audio | Masse |
|------|----------------------------------|-------|--------|
| **Funk A** ([RJ45](https://de.wikipedia.org/wiki/RJ-45)) | **U1 [ISO7741](https://www.ti.com/product/ISO7741)** ([UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle) digital isolator) | **T1/T2** [Audio-Trafos](https://de.wikipedia.org/wiki/Transformator) 1:1 | **GND_RADIO_A** ≠ GND_ESP |
| **Funk B** (J5) | **U6 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** ([USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) isolator) | im [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) (UAC) | **GND_USB_B** |
| **Funk C** (J6) | **U7 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** | im [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) (UAC) | **GND_USB_C** |
| **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** | — | [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) nur ESP-Seite der Trafos | **GND_ESP** |

- **Kein** direkter Kupferverbund zwischen `GND_RADIO_A`, `GND_USB_B`, `GND_USB_C` und `GND_ESP`.
- **[USB2422](https://www.microchip.com/en-us/product/USB2422) (U5)** sitzt nur auf der **ESP-Seite**; je Downstream-Port ein **eigener [ADuM4160](https://www.analog.com/en/products/adum4160.html)** + **isoliertes DC/DC (U8/U9)** für [VBUS](https://de.wikipedia.org/wiki/Universal_Serial_Bus).
- [RJ45](https://de.wikipedia.org/wiki/RJ-45) **Pin 2/7** → `GND_RADIO_A` (nicht ESP-Masse). Gehäuse über **RC** (z. B. 1 nF + 1 MΩ) optional zu Funk-Masse, nicht hart zu ESP.
- **+5 V**: getrennte Versorgung empfohlen — ESP über eigenes [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)/[WiFi](https://de.wikipedia.org/wiki/WLAN)-Board; Funk A +5 V nur für isolierte Sekundärseite / Trafos.

### Wechsel zwischen [Funkgeräten](https://de.wikipedia.org/wiki/Transceiver)

Firmware schaltet **logisch** ([rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html)-Kanal, [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Index). **Elektrisch** bleiben die Massefelder getrennt — kein Relais, das GNDs zusammenlegt.

### Dokumentation Hardware

- [SCHEMATIC.md](../hardware/esp32-flrig-shield/docs/SCHEMATIC.md) (Rev C Netze)
- [ASSEMBLY.md](../hardware/esp32-flrig-shield/docs/ASSEMBLY.md) (Prüfpunkte)

<a id="english"></a>

## English

### Issue

Without isolation, **ESP ground**, **radio A ground** ([RJ45](https://en.wikipedia.org/wiki/Registered_jack)), and **[USB](https://en.wikipedia.org/wiki/USB) B/C grounds** are tied through cables and shields → **[ground loops](https://en.wikipedia.org/wiki/Ground_loop_(electricity))**, hum, [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) corruption when multiple rigs are connected.

### Solution (Rev C)

| Path | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)) | Audio | Ground |
|------|----------------------------------|-------|--------|
| **Radio A** ([RJ45](https://en.wikipedia.org/wiki/Registered_jack)) | **U1 [ISO7741](https://www.ti.com/product/ISO7741)** digital isolator | **T1/T2** 1:1 [audio transformers](https://en.wikipedia.org/wiki/Transformer) | **GND_RADIO_A** ≠ GND_ESP |
| **Radio B** (J5) | **U6 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** [USB](https://en.wikipedia.org/wiki/USB) isolator | via [USB UAC](https://en.wikipedia.org/wiki/USB_audio_device_class) | **GND_USB_B** |
| **Radio C** (J6) | **U7 [ADuM4160](https://www.analog.com/en/products/adum4160.html)** | via [USB UAC](https://en.wikipedia.org/wiki/USB_audio_device_class) | **GND_USB_C** |
| **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** | — | [I2S](https://en.wikipedia.org/wiki/I%C2%B2S) on ESP side of transformers only | **GND_ESP** |

- **No** direct copper between radio/[USB](https://en.wikipedia.org/wiki/USB) grounds and ESP ground.
- **[USB2422](https://www.microchip.com/en-us/product/USB2422)** on ESP side only; **one [ADuM4160](https://www.analog.com/en/products/adum4160.html) per** [USB-A](https://en.wikipedia.org/wiki/USB) port plus **isolated DC/DC** for [VBUS](https://en.wikipedia.org/wiki/USB).
- [RJ45](https://en.wikipedia.org/wiki/Registered_jack) pins **2/7** → `GND_RADIO_A` only.
- Prefer **separate** +5 V feeds: ESP from its module; radio A +5 V for isolated secondary only.

### [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) acronym

**[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** = **[Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** ([ICOM](https://en.wikipedia.org/wiki/Icom)/[Yaesu](https://en.wikipedia.org/wiki/Yaesu_(brand)) control protocol), not related to any construction-equipment brand.
