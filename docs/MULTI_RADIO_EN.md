# 3× Transceiver + Rotor (parallel over WiFi)

[Deutsch](MULTI_RADIO_DE.md)

## System (Shield Rev B)

| Channel | Hardware | CAT | Audio → PC |
|---------|----------|-----|------------|
| **A** | **RJ45 J1** | analog UART | I2S → UDP **4533/4534**, `/audio` |
| **B** | **USB-A J5** | USB CDC | USB UAC → **4538/4539**, `/audio_b` |
| **C** | **USB-A J6** | USB CDC | USB UAC → **4541/4542**, `/audio_c` |
| **Rotor** | **J7** / **J3** | — | **rotctld** port **4535** |

All three radios can run at the same time with separate **rigctld** TCP ports and audio UDP pairs over **WiFi**.

## Firmware

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

```bash
rigctl -m 2 -r <IP>:4532 f   # A (RJ45)
rigctl -m 2 -r <IP>:4536 f   # B (USB J5)
rigctl -m 2 -r <IP>:4540 f   # C (USB J6)
rotctl -m 2 -r <IP>:4535 p   # Rotor
```

## Power

- **RJ45 pin 1** (+5 V) or external supply feeds the shield, **USB2422** hub, and **VBUS** on J5/J6.
- Use **≥2 A** at +5 V when two USB radios draw power simultaneously.

## PCB

- Rev **B**: 100×72 mm, **two USB-A female host ports**, **USB2422** hub, **J3** ESP32-S3 OTG + rotor GPIO.
- Gerbers: [`hardware/esp32-flrig-shield/fabrication/esp32-flrig-shield-jlc.zip`](../hardware/esp32-flrig-shield/fabrication/esp32-flrig-shield-jlc.zip)

## CYD note

The **Cheap Yellow Display (ESP32)** has **no USB host**. Rev B USB ports require **ESP32-S3** on **J3**. On CYD only **channel A (RJ45)** + rotor (wire **J7** to GPIO) are supported.
