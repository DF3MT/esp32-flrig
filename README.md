# ESP32 CAT Remote Panel

ESP32-basiertes Funkfernbedienungs-Panel mit Touch-Display, 5 programmierbaren Potentiometern und universeller **ICOM CI-V** / **Yaesu CAT**-Steuerung. Kompatibel mit **[flrig](https://github.com/w1hkj/flrig)** und **[Hamlib rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html)** über das standardisierte rigctl-TCP-Protokoll (Port 4532).

## Architektur

```
┌─────────────┐   WiFi/TCP:4532    ┌──────────────┐
│ flrig/fldigi│◄──────────────────►│   ESP32      │
│ WSJT-X etc. │   rigctl-Protokoll│  rigctld     │
└─────────────┘                    │  Server      │
                                   ├──────┬───────┤
                                   │ LVGL │ 5x    │
                                   │ Touch│ Potis │
                                   └──────┴───┬───┘
                                              │ UART (CAT)
                                         ┌────▼────┐
                                         │ ICOM /  │
                                         │ YAESU   │
                                         └─────────┘
```

### Betriebsmodi

| Modus | Beschreibung |
|-------|-------------|
| **DIRECT_CAT** (Standard) | ESP32 steuert das Funkgerät direkt per UART |
| **Client → flrig** | ESP32 als rigctl-Client zu PC mit flrig (Modell 4, Port 12345) |
| **Client → rigctld** | ESP32 als Client zu Hamlib rigctld auf dem PC |

## Hardware

### Empfohlen: ESP32-2432S028 (Cheap Yellow Display)

| Funktion | GPIO |
|----------|------|
| CAT RX | 16 |
| CAT TX | 17 |
| Poti 1–5 | 32, 35, 34, 39, 36 |
| Display/Touch | onboard (ILI9341 + XPT2046) |

### CAT-Verkabelung

```
ESP32          Funkgerät (CAT/CI-V)
GPIO17 (TX) ──► RX (über Level-Shifter 3,3V↔5V)
GPIO16 (RX) ◄── TX
GND ──────────► GND
```

> **Wichtig:** Die meisten ICOM/Yaesu-CAT-Ports sind 5V-TTL. Ein Bidirektional-Level-Shifter (z.B. TXS0108E) ist empfohlen.

### Potentiometer

5× 10 kΩ Potentiometer, Mittelablauf an GPIO, Außenleitungen an 3,3 V und GND.

## Software bauen & flashen

Voraussetzungen: [PlatformIO](https://platformio.org/)

```bash
# Python venv einrichten (empfohlen)
python3 -m venv .venv
source .venv/bin/activate
pip install platformio

# CYD-Board (Standard)
pio run -e esp32-cyd -t upload

# Generisches ILI9341-Board
pio run -e esp32-generic -t upload

# Filesystem (config.json) hochladen
pio run -e esp32-cyd -t uploadfs

# Serial Monitor
pio device monitor
```

### WSL2 unter Windows

USB-Serial ist in WSL2 **nicht nativ** verfügbar. Das Board erscheint unter Windows als COM-Port (z.B. `COM5`).

```bash
# Build in WSL, Flash ueber Windows (empfohlen)
./scripts/flash.sh esp32-tdisplay
ESP_PORT=COM5 ./scripts/flash.sh esp32-tdisplay
```

Alternativ: [usbipd-win](https://learn.microsoft.com/en-us/windows/wsl/connect-usb) installieren und USB an WSL durchreichen, dann normal `pio run -t upload`.

### TTGO T-Display v1.1

Environment: `esp32-tdisplay` – ST7789 135x240, zwei Tasten statt Touch.

| Funktion | GPIO |
|----------|------|
| CAT RX / TX | 27 / 17 |
| Poti 1–5 | 32, 33, 25, 26, 34 |
| Taste +1 kHz | 35 |
| Taste −1 kHz | 0 (BOOT) |

## Erstinbetriebnahme

1. ESP32 flashen und einschalten
2. WiFi-AP erscheint: **`ESP32-CAT-Panel`** / Passwort: **`hamradio123`**
3. Browser öffnen: **http://192.168.4.1**
4. Hersteller (ICOM/YAESU), CI-V-Adresse, Baudrate und Poti-Mapping konfigurieren
5. Speichern → Neustart

## Integration mit flrig / Hamlib

### ESP32 als rigctld-Server (PC steuert ESP32/Radio)

Auf dem PC:

```bash
# Hamlib – direkt zum ESP32
rigctl -m 2 -r 192.168.4.1:4532 f      # Frequenz lesen
rigctl -m 2 -r 192.168.4.1:4532 F 14200000  # Frequenz setzen
```

In **flrig**: Rig → Hamlib → Modell **NET rigctl (2)**, Device: `192.168.x.x:4532`

### PC mit flrig als Server (ESP32 als Client)

1. flrig starten, XML-RPC-Server aktivieren (Port **12345**)
2. Im Web-UI des ESP32: Remote Host = PC-IP, Port = 12345
3. Alternativ auf dem PC:

```bash
rigctld -m 4 -r 192.168.x.x:12345 -t 4532
```

Dann verbinden andere Programme mit `localhost:4532`.

Referenz: [flrig XML-RPC Server](https://www.w1hkj.org/flrig-help/xmlrpc_server.html)

## Potentiometer – frei programmierbar

Jeder der 5 Potis kann über Web-UI oder `/config.json` konfiguriert werden:

| Aktion | Wirkung |
|--------|---------|
| `FREQ_COARSE` | Grobe Frequenzänderung (± kHz) |
| `FREQ_FINE` | Feine Frequenzänderung (± Hz) |
| `AF_GAIN` | AF-Lautstärke (0.0–1.0) |
| `RF_POWER` | Sendeleistung |
| `RF_GAIN` | RF-Gain / AGC |
| `SQUELCH` | Squelch |
| `MIC_GAIN` | Mikrofonverstärkung |
| `RIT_OFFSET` | RIT-Offset |
| `CUSTOM_RIGCTL` | Eigener Befehl mit `{val}`-Platzhalter |

Beispiel `config.json`:

```json
{
  "pots": [
    {"action": "FREQ_COARSE", "min": -100000, "max": 100000, "step": 10000},
    {"action": "FREQ_FINE",   "min": -5000,   "max": 5000,   "step": 10},
    {"action": "AF_GAIN",     "min": 0.0,     "max": 1.0,    "step": 0.01},
    {"action": "RF_POWER",    "min": 0.0,     "max": 1.0,    "step": 0.01},
    {"action": "SQUELCH",     "min": 0.0,     "max": 1.0,    "step": 0.01}
  ]
}
```

## Unterstützte CAT-Befehle (rigctld)

| Befehl | Funktion |
|--------|----------|
| `f` / `F` | Frequenz lesen/setzen |
| `m` / `M` | Modus lesen/setzen |
| `t` / `T` | PTT lesen/setzen |
| `l` / `L` | Level lesen/setzen (AF, RFPOWER) |
| `w` | Raw CI-V/CAT (Hex) |
| `\get_info` | Geräteinfo |

## Touch-Display UI

- Frequenzanzeige (MHz)
- Modus (USB/LSB/CW/…)
- RX/TX-Status
- Tasten: +1 kHz / −1 kHz

## ICOM CI-V Adressen (Beispiele)

| Radio | Adresse (hex) |
|-------|---------------|
| IC-7300 | 0x94 |
| IC-705 | 0xA4 |
| IC-7610 | 0x98 |

## Yaesu CAT

Standard-ASCII-Befehle (`FA`, `MD`, `PC`, `AG`, `SQ`, `TX`). Baudrate meist **38400** (je nach Modell auch 4800 oder 9600).

## Projektstruktur

```
├── platformio.ini          # Build-Konfiguration
├── include/                # Header
│   ├── config.h            # Pins, Enums
│   ├── icom_civ.h          # ICOM CI-V
│   ├── yaesu_cat.h         # Yaesu CAT
│   ├── cat_controller.h    # Unified CAT API
│   ├── rigctld_server.h    # Hamlib TCP server
│   ├── pot_manager.h       # Potentiometer logic
│   └── touch_ui.h          # LVGL UI
├── src/                    # Implementierung
└── data/config.json        # Default-Konfiguration
```

## Lizenz

GPL-2.0 (kompatibel mit flrig/Hamlib-Ökosystem)

## Weiterführende Links

- [flrig auf GitHub](https://github.com/w1hkj/flrig)
- [Hamlib rigctld Manual](https://hamlib.sourceforge.net/html/rigctld.1.html)
- [flrig XML-RPC Hilfe](https://www.w1hkj.org/flrig-help/xmlrpc_server.html)
# esp32-flrig
