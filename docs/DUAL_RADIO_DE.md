# Zwei Funkgeräte gleichzeitig (CAT + Audio)

> **3× Funk + Rotor (Shield Rev B):** siehe [MULTI_RADIO_DE.md](MULTI_RADIO_DE.md)

## Übersicht

| Build | Hardware | CAT | Audio |
|-------|----------|-----|-------|
| `esp32-cyd` | CYD / Shield RJ45 | 1× UART | 1× I2S |
| `esp32-cyd-dual` | CYD + 2. UART (GPIO5/18) | 2× UART | 2× I2S (GPIO12–15) |
| `esp32-s3-dual-usb` | ESP32-S3 + **Powered USB-Hub** | 2× USB-CDC | 2× USB-UAC (geplant) oder I2S |

Hamlib pro Funk:

- **A:** `rigctl -m 2 -r <IP>:4532` · Audio UDP 4533/4534 · Web `/audio`
- **B:** `rigctl -m 2 -r <IP>:4536` · Audio UDP 4538/4539 · Web `/audio_b`

Rotor unverändert: `rotctld` Port **4535**.

## USB am ESP32 (S3)

Der **ESP32 (CYD)** hat keinen USB-Host – nur USB-Serial zum PC (CH340).

Für **2× Funk per USB-Kabel** (volles CAT + USB-Audio des Geräts):

1. Board **ESP32-S3** mit OTG (z. B. S3-DevKitC-1 + OTG-Adapter).
2. **Aktiver USB-2.0-Hub** mit Netzteil (Strom für 2× Funk).
3. Firmware: `pio run -t upload -e esp32-s3-dual-usb`
4. In `config.json` pro Kanal: `"cat_link":"usb"`, `"audio_link":"usb"`, `"usb_index":0` bzw. `1`.

USB-CDC-Host-Treiber ist vorbereitet (`usb_cat_host.cpp`); nach Attach erscheinen die CAT-Ports. **USB-Audio (UAC)** folgt im gleichen Host-Stack – bis dahin Audio über Shield-I2S/RJ45 oder WiFi-UDP.

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

## WSJT-X / FT8 (2 Rigs)

Zwei Instanzen oder Multi-Instance: jeweils eigenes `rigctl`-Host/Port und Audio-UDP-Paar (4533/4534 und 4538/4539). Siehe `scripts/ft8_setup.py` – zweites Profil mit Port 4536 ergänzen.
