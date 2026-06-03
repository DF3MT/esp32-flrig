# 3× radios in parallel — independent on the PC

[Deutsch](MULTI_RADIO_DE.md) · [Glossary](GLOSSARY.md#english)

## What “3 radios” means

On the **PC**, you run **three separate control and audio chains** — not one mixed stream:

| On PC | Radio A | Radio B | Radio C |
|-------|---------|---------|---------|
| **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html))** | [TCP](https://en.wikipedia.org/wiki/Transmission_Control_Protocol) `<IP>`:**4532** | [TCP](https://en.wikipedia.org/wiki/Transmission_Control_Protocol) `<IP>`:**4536** | [TCP](https://en.wikipedia.org/wiki/Transmission_Control_Protocol) `<IP>`:**4540** |
| **Audio RX** (radio → PC) | [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) **4533** | [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) **4538** | [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) **4541** |
| **Audio TX** (PC → radio) | [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) **4534** | [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) **4539** | [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) **4542** |
| **Web audio** (optional) | `/audio` | `/audio_b` | `/audio_c` |
| **[Rotor](https://en.wikipedia.org/wiki/Antenna_rotator)** (shared) | — | — | — |

- The [ESP32](https://en.wikipedia.org/wiki/ESP32) runs **three independent `rigctld` servers** and **three `AudioBridge` tasks** (separate UDP sockets, no summing/mixing).
- Each PC application must use its **own [rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html) host:port** and **own audio [UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) port pair** (e.g. three [WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html) instances with separate virtual audio cables).

**[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** = [Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver). **Ground:** [galvanically isolated](https://en.wikipedia.org/wiki/Galvanic_isolation) ([ISOLATION.md](ISOLATION.md)).

## Example on the PC (3× [FT8](https://en.wikipedia.org/wiki/FT8_(digital_mode)))

```
┌──────────────── PC ─────────────────────────────────────────────┐
│  WSJT-X #1          WSJT-X #2          fldigi / other app        │
│  rigctl → :4532     rigctl → :4536     rigctl → :4540            │
│  Audio 4533/4534    Audio 4538/4539    Audio 4541/4542           │
│  VB-Cable A         VB-Cable B         VB-Cable C                │
└────────┬──────────────────┬──────────────────┬──────────────────┘
         │ WiFi             │                  │
         ▼                  ▼                  ▼
    ┌──────── ESP32 (3 parallel channels, not mixed) ───────────────┐
    │  Ch A (RJ45)        Ch B (USB J5)        Ch C (USB J6)         │
    └────────────────────────────────────────────────────────────────┘
```

| Instance | [rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html) | Audio bridge (script) | Typical PC audio |
|----------|--------|----------------------|------------------|
| 1 | `-r 192.168.4.1:4532` | `audio_client.py --port-out 4533 --port-in 4534` | [VB-Audio](https://vb-audio.com/Cable/) Cable **A** |
| 2 | `-r 192.168.4.1:4536` | `--port-out 4538 --port-in 4539` | [VB-Audio](https://vb-audio.com/Cable/) Cable **B** |
| 3 | `-r 192.168.4.1:4540` | `--port-out 4541 --port-in 4542` | [VB-Audio](https://vb-audio.com/Cable/) Cable **C** |

`scripts/ft8_setup.py` sets up **one** instance; for three radios use **three profiles** (separate [JSON](https://en.wikipedia.org/wiki/JSON) + separate virtual cable per app).

## Hardware (Shield Rev C)

| Channel | Connector | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | Audio to ESP |
|---------|-----------|-----|--------------|
| **A** | [RJ45](https://en.wikipedia.org/wiki/Registered_jack) | [UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter) (isolated) | [I2S](https://en.wikipedia.org/wiki/I%C2%B2S) |
| **B** | [USB-A](https://en.wikipedia.org/wiki/USB) J5 | [USB CDC](https://en.wikipedia.org/wiki/USB_communications_device_class) | [USB UAC](https://en.wikipedia.org/wiki/USB_audio_device_class) (S3) |
| **C** | [USB](https://en.wikipedia.org/wiki/USB)-A J6 | [USB CDC](https://en.wikipedia.org/wiki/USB_communications_device_class) | [USB UAC](https://en.wikipedia.org/wiki/USB_audio_device_class) (S3) |
| **[Rotor](https://en.wikipedia.org/wiki/Antenna_rotator)** | J7 | — | [rotctld](https://hamlib.sourceforge.net/html/rotctld.1.html) **4535** |

## Firmware & config

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

Enable all three channels in `config.json` (`radios[]` — see [ft8_config.example-3radio.json](../scripts/ft8_config.example-3radio.json)):

```bash
rigctl -m 2 -r <IP>:4532 f
rigctl -m 2 -r <IP>:4536 f
rigctl -m 2 -r <IP>:4540 f
rotctl -m 2 -r <IP>:4535 p
```

## Important

| Topic | Behavior |
|-------|----------|
| Audio mixing on ESP | **No** — per-channel task and ports |
| One [rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html) for all | **No** — one [TCP](https://en.wikipedia.org/wiki/Transmission_Control_Protocol) port per radio |
| One [VB-Cable](https://vb-audio.com/Cable/) for all | **No** — one virtual device per PC app recommended |
| [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) (no S3) | **Channel A** + [rotor](https://en.wikipedia.org/wiki/Antenna_rotator) only; B/C need [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) + shield |

More: [Shield README](../hardware/esp32-flrig-shield/README.md) · [main README](../README.md)
