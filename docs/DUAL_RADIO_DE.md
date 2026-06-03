# Zwei [Funkgeräte](https://de.wikipedia.org/wiki/Transceiver) gleichzeitig ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + Audio)

> **3× Funk + [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)) (Shield Rev B):** siehe [MULTI_RADIO_DE.md](MULTI_RADIO_DE.md)

## Übersicht

| Build | Hardware | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | Audio |
|-------|----------|-----|-------|
| `esp32-cyd` | [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) / Shield [RJ45](https://de.wikipedia.org/wiki/RJ-45) | 1× [UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle) | 1× [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) |
| `esp32-cyd-dual` | [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) + 2. [UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle) (GPIO5/18) | 2× [UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle) | 2× [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) (GPIO12–15) |
| `esp32-s3-dual-usb` | [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) + **Powered [USB-Hub](https://de.wikipedia.org/wiki/USB-Hub)** | 2× [USB-CDC](https://de.wikipedia.org/wiki/USB_Communications_Device_Class) | 2× [USB-UAC](https://de.wikipedia.org/wiki/USB-Audioklasse) (geplant) oder [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) |

[Hamlib](https://en.wikipedia.org/wiki/Hamlib) pro Funk:

- **A:** `rigctl -m 2 -r <IP>:4532` · Audio [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) 4533/4534 · Web `/audio`
- **B:** `rigctl -m 2 -r <IP>:4536` · Audio [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) 4538/4539 · Web `/audio_b`

[Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)) unverändert: `rotctld` Port **4535**.

## [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus) am [ESP32](https://de.wikipedia.org/wiki/ESP32) (S3)

Der **[ESP32](https://de.wikipedia.org/wiki/ESP32) ([CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display))** hat keinen [USB-Host](https://de.wikipedia.org/wiki/Universal_Serial_Bus) – nur [USB-Serial](https://de.wikipedia.org/wiki/USB_Communications_Device_Class) zum PC ([CH340](https://de.wikipedia.org/wiki/USB-UART-Bridge)).

Für **2× Funk per [USB-Kabel](https://de.wikipedia.org/wiki/USB)** (volles [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + [USB-Audio](https://de.wikipedia.org/wiki/USB-Audioklasse) des Geräts):

1. Board **[ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3)** mit OTG (z. B. S3-DevKitC-1 + OTG-Adapter).
2. **Aktiver [USB-2.0](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Hub** mit Netzteil (Strom für 2× Funk).
3. Firmware: `pio run -t upload -e esp32-s3-dual-usb`
4. In `config.json` pro Kanal: `"cat_link":"usb"`, `"audio_link":"usb"`, `"usb_index":0` bzw. `1`.

[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-CDC-Host-Treiber ist vorbereitet (`usb_cat_host.cpp`); nach Attach erscheinen die [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Ports. **[USB-Audio](https://de.wikipedia.org/wiki/USB-Audioklasse) (UAC)** folgt im gleichen Host-Stack – bis dahin Audio über Shield-[I2S](https://de.wikipedia.org/wiki/I%C2%B2S)/[RJ45](https://de.wikipedia.org/wiki/RJ-45) oder [WiFi](https://de.wikipedia.org/wiki/WLAN)-[UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol).

## Konfiguration (`radios[]`)

```json
"radios": [
  {
    "enabled": true,
    "label": "A",
    "radio_model": "G90",
    "cat_link": "uart",
    "audio_link": "i2s",
    "audio_enabled": true,
    "rigctld_port": 4532,
    "audio_port_out": 4533,
    "audio_port_in": 4534
  },
  {
    "enabled": true,
    "label": "B",
    "radio_model": "FT-991A",
    "cat_link": "usb",
    "audio_link": "usb",
    "usb_index": 1,
    "rigctld_port": 4536,
    "audio_port_out": 4538,
    "audio_port_in": 4539
  }
]
```

Legacy-Felder (`radio_model`, `audio_enabled`, …) spiegeln weiterhin **Kanal A**.

## [WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html) / [FT8](https://de.wikipedia.org/wiki/FT8_(Amateurfunk)) (2 Rigs)

Zwei Instanzen oder Multi-Instance: jeweils eigenes `rigctl`-Host/Port und Audio-[UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol)-Paar (4533/4534 und 4538/4539). Siehe `scripts/ft8_setup.py` – zweites Profil mit Port 4536 ergänzen.
