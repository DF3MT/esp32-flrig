# 3× Transceiver + Rotor (Parallel über WiFi)

[English](MULTI_RADIO_EN.md)

## System (Shield Rev B)

| Kanal | Hardware | CAT | Audio → PC |
|-------|----------|-----|------------|
| **A** | RJ45 (analog CAT + Line) | UART / CI-V / Yaesu | I2S → UDP **4533/4534**, Web `/audio` |
| **B** | **USB-A Buchse J5** | USB-CDC (Host) | USB-UAC → UDP **4538/4539**, `/audio_b` |
| **C** | **USB-A Buchse J6** | USB-CDC (Host) | USB-UAC → UDP **4541/4542**, `/audio_c` |
| **Rotor** | J7 / J3 (GPIO) | — | **rotctld** Port **4535** |

Alle drei Funkgeräte gleichzeitig: je ein **rigctld**-TCP-Port und ein Audio-UDP-Paar über **WLAN** zum PC (WSJT-X, fldigi, zwei weitere CAT-Clients).

**Masse:** Shield **Rev C** — keine direkte GND-Verbindung zwischen den Funkgeräten ([ISOLATION.md](ISOLATION.md)). **CAT** = **Computer Aided Transceiver**.

```
                    ┌──────────────── ESP32-S3 + Shield Rev B ────────────────┐
  RJ45 Funk A ──────┤ J1  CAT+Audio analog                                      │
  USB Kabel B ──────┤ J5  USB-A female ── U5 USB2422 Hub ── J3 OTG ← S3        │
  USB Kabel C ──────┤ J6  USB-A female ──┘                                      │
  Rotor Relais ─────┤ J7  Taster + OC (parallel zu Funkbetrieb)                 │
                    │ WiFi: :4532 :4536 :4540  rotctld :4535                     │
                    └──────────────────────────────────────────────────────────┘
```

## Firmware

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

Hamlib:

```bash
rigctl -m 2 -r <IP>:4532 f   # Funk A (RJ45)
rigctl -m 2 -r <IP>:4536 f   # Funk B (USB J5)
rigctl -m 2 -r <IP>:4540 f   # Funk C (USB J6)
rotctl -m 2 -r <IP>:4535 p   # Rotor
```

## Stromversorgung

- **RJ45 Pin 1** (+5 V vom Funk A oder extern) speist Shield, Hub **U5** und **VBUS** an J5/J6.
- Zusätzlich **≥2 A** empfohlen, wenn zwei USB-Funkgerät gleichzeitig laden/Audio ziehen.
- Optional separates Netzteil an **+5V_BUS** (siehe [SCHEMATIC.md](../hardware/esp32-flrig-shield/docs/SCHEMATIC.md)).

## Platine

- Rev **B**: 100×72 mm, **2× USB-A Buchse weiblich**, Hub **USB2422**, **J3** ESP32-S3 OTG + Rotor-GPIO.
- Details: [hardware/esp32-flrig-shield/README.md](../hardware/esp32-flrig-shield/README.md)

## CYD-Hinweis

Der **Cheap Yellow Display (ESP32)** hat **keinen** USB-Host. Rev-B-Shield mit USB-Ports erfordert **ESP32-S3** am J3. Am CYD nutzbar: nur **Kanal A** (RJ45) + Rotor (J7 mit Drahtbrücken zu GPIO).
