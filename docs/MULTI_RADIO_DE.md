# 3× Funk parallel — unabhängig am PC

[English](MULTI_RADIO_EN.md) · [Glossar / Fachbegriffe](GLOSSARY.md#deutsch)

## Was „3 [Funkgeräte](https://de.wikipedia.org/wiki/Transceiver)“ bedeutet

Am **PC** laufen **drei getrennte Steuer- und Audio-Ketten** — nicht ein gemischter Stream:

| Am PC | Funk A | Funk B | Funk C |
|--------|--------|--------|--------|
| **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) ([rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html))** | [TCP](https://de.wikipedia.org/wiki/Transmission_Control_Protocol) `<IP>`:**4532** | [TCP](https://de.wikipedia.org/wiki/Transmission_Control_Protocol) `<IP>`:**4536** | [TCP](https://de.wikipedia.org/wiki/Transmission_Control_Protocol) `<IP>`:**4540** |
| **Audio RX** (Funk → PC) | [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4533** | [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4538** | [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4541** |
| **Audio TX** (PC → Funk) | [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4534** | [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4539** | [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4542** |
| **Web-Audio** (optional) | `/audio` | `/audio_b` | `/audio_c` |
| **[Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))** (einmalig) | — | — | — |

- Der [ESP32](https://de.wikipedia.org/wiki/ESP32) betreibt **drei unabhängige `rigctld`-Server** und **drei `AudioBridge`-Tasks** (eigene UDP-Sockets, kein Summieren/Mixing).
- Jede PC-Anwendung muss **eigenes [rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html)-Host:Port** und **eigenes Audio-[UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol)-Paar** nutzen (z. B. drei [WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html)-Instanzen mit je einem virtuellen Kabel).

**[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)** = [Computer Aided Transceiver](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver). **Masse:** [galvanisch getrennt](https://de.wikipedia.org/wiki/Galvanische_Trennung) ([ISOLATION.md](ISOLATION.md)).

## Beispiel am PC (3× [FT8](https://de.wikipedia.org/wiki/FT8_(Amateurfunk)))

```
┌──────────────── PC ─────────────────────────────────────────────┐
│  WSJT-X #1          WSJT-X #2          fldigi / anderes Programm │
│  rigctl → :4532     rigctl → :4536     rigctl → :4540            │
│  Audio 4533/4534    Audio 4538/4539    Audio 4541/4542           │
│  VB-Cable A         VB-Cable B         VB-Cable C                │
└────────┬──────────────────┬──────────────────┬──────────────────┘
         │ WiFi             │                  │
         ▼                  ▼                  ▼
    ┌──────── ESP32 (3 Kanäle parallel, nicht gemischt) ────────────┐
    │  Kanal A (RJ45)     Kanal B (USB J5)     Kanal C (USB J6)      │
    └────────────────────────────────────────────────────────────────┘
```

| Instanz | `rigctld` / [rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html) | Audio-Bridge (Skript) | Typisches PC-Audio |
|---------|-------------------|------------------------|-------------------|
| 1 | `-r 192.168.4.1:4532` | `audio_client.py --port-out 4533 --port-in 4534` | [VB-Audio](https://vb-audio.com/Cable/) Cable **A** |
| 2 | `-r 192.168.4.1:4536` | `--port-out 4538 --port-in 4539` | [VB-Audio](https://vb-audio.com/Cable/) Cable **B** |
| 3 | `-r 192.168.4.1:4540` | `--port-out 4541 --port-in 4542` | [VB-Audio](https://vb-audio.com/Cable/) Cable **C** |

`scripts/ft8_setup.py` konfiguriert **eine** Instanz; für drei [Funkgeräte](https://de.wikipedia.org/wiki/Transceiver) **drei Profile** (eigene [JSON](https://de.wikipedia.org/wiki/JSON) + eigenes virtuelles Kabel pro Instanz).

## Hardware (Shield Rev C)

| Kanal | Anschluss | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | Audio zum ESP |
|-------|-----------|-----|----------------|
| **A** | [RJ45](https://de.wikipedia.org/wiki/RJ-45) | [UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle) (isoliert) | [I2S](https://de.wikipedia.org/wiki/I%C2%B2S) |
| **B** | [USB-A](https://de.wikipedia.org/wiki/USB) J5 | [USB-CDC](https://de.wikipedia.org/wiki/USB_Communications_Device_Class) | [USB-UAC](https://de.wikipedia.org/wiki/USB-Audioklasse) (S3) |
| **C** | [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-A J6 | [USB-CDC](https://de.wikipedia.org/wiki/USB_Communications_Device_Class) | [USB-UAC](https://de.wikipedia.org/wiki/USB-Audioklasse) (S3) |
| **[Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne))** | J7 | — | [rotctld](https://hamlib.sourceforge.net/html/rotctld.1.html) **4535** |

```
  Funk A ── RJ45 ── ESP Kanal A ── WiFi ── PC App 1
  Funk B ── USB J5 ── ESP Kanal B ── WiFi ── PC App 2
  Funk C ── USB J6 ── ESP Kanal C ── WiFi ── PC App 3
```

## Firmware & Konfiguration

```bash
pio run -t upload -e esp32-flrig-shield
pio run -t uploadfs -e esp32-flrig-shield
```

In `config.json` alle drei Kanäle aktivieren (`radios[]`, siehe Beispiel in [ft8_config.example-3radio.json](../scripts/ft8_config.example-3radio.json)):

```bash
rigctl -m 2 -r <IP>:4532 f
rigctl -m 2 -r <IP>:4536 f
rigctl -m 2 -r <IP>:4540 f
rotctl -m 2 -r <IP>:4535 p
```

## Wichtig

| Thema | Verhalten |
|-------|-----------|
| Audio-Mixing auf ESP | **Nein** — je Kanal eigener Task + Ports |
| Ein [rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html) für alle | **Nein** — je Funk eigener [TCP](https://de.wikipedia.org/wiki/Transmission_Control_Protocol)-Port |
| Ein [VB-Cable](https://vb-audio.com/Cable/) für alle | **Nein** — je PC-Programm eigenes Gerät empfohlen |
| [CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) (ohne S3) | Nur **Kanal A** + [Rotor](https://de.wikipedia.org/wiki/Rotor_(Antenne)); B/C brauchen [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3) + Shield |

Weitere Infos: [Shield README](../hardware/esp32-flrig-shield/README.md) · [Haupt-README](../README.md)
