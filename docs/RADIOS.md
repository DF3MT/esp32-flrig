# Unterstützte [Funkgeräte](https://de.wikipedia.org/wiki/Transceiver) ([CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) + Audio)

Das [ESP32](https://de.wikipedia.org/wiki/ESP32)-Panel spricht **direkt per [UART](https://de.wikipedia.org/wiki/Asynchrone_Serielle_Schnittstelle)** mit dem [Funkgerät](https://de.wikipedia.org/wiki/Transceiver) (3,3 V [TTL](https://de.wikipedia.org/wiki/Transistor-Transistor-Logik) über [Pegelwandler](https://de.wikipedia.org/wiki/Pegelwandler)). In der Web-Konfiguration (`http://<ESP-IP>/`) wählen Sie das Modell – **Baudrate, Protokoll und [CI-V](https://de.wikipedia.org/wiki/CI-V)-Adresse** werden automatisch gesetzt.

## Übersicht

| Profil-ID | Gerät | [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Protokoll | Baud | [CI-V](https://de.wikipedia.org/wiki/CI-V) Addr |
|-----------|-------|---------------|------|-----------|
| `XIEGU_G90` | [Xiegu](https://www.xiegu.com/) G90 | [ICOM](https://de.wikipedia.org/wiki/Icom) [CI-V](https://de.wikipedia.org/wiki/CI-V) | 19200 | A4 |
| `XIEGU_X6100` | [Xiegu](https://www.xiegu.com/) [X6100](https://www.xiegu.com/) | [ICOM](https://de.wikipedia.org/wiki/Icom) [CI-V](https://de.wikipedia.org/wiki/CI-V) | 19200 | A4 |
| `XIEGU_X6200` | [Xiegu](https://www.xiegu.com/) [X6200](https://www.xiegu.com/) | [ICOM](https://de.wikipedia.org/wiki/Icom) [CI-V](https://de.wikipedia.org/wiki/CI-V) | 19200 | A4 |
| `YAESU_FT991A` | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-991A | [Yaesu CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 38400 | — |
| `YAESU_FT910` | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-910 | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 38400 | — |
| `YAESU_FTDX10` | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-DX10 | [Yaesu CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 38400 | — |
| `YAESU_FTDX101D` | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-DX101D | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 38400 | — |
| `YAESU_FTDX101MP` | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-DX101MP | [Yaesu CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 38400 | — |
| `YAESU_FT891` | Yaesu FT-891 | [Yaesu CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 38400 | — |
| `YAESU_FT897` | Yaesu FT-897 | [Yaesu CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 4800* | — |
| `YAESU_FT857` | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) FT-857 | [Yaesu](https://de.wikipedia.org/wiki/Yaesu) [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) | 4800 | — |

\* FT-897: Menü-[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Rate prüfen (oft 4800, manche 38400).

## [Xiegu](https://www.xiegu.com/) (G90, [X6100](https://www.xiegu.com/), [X6200](https://www.xiegu.com/))

- **Protokoll:** [CI-V](https://de.wikipedia.org/wiki/CI-V)-kompatibel (wie [ICOM](https://de.wikipedia.org/wiki/Icom)), **nicht** [Yaesu](https://de.wikipedia.org/wiki/Yaesu)-[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver).
- **Einstellungen:** 19200 [bps](https://de.wikipedia.org/wiki/Bit_pro_Sekunde), Radio-Adresse **0xA4** (wird vom Profil gesetzt).
- **Anschluss G90:** [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) nur an der **Display-Einheit** (3,5 mm / IO-Port), nicht am Basisgerät.
- **[X6100](https://www.xiegu.com/)/[X6200](https://www.xiegu.com/):** [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-C DEV, Serial-B für [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver); [USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)-Audio für Sound.
- **[WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html) / [flrig](https://github.com/w1hkj/flrig):** Modell z. B. „[Xiegu](https://www.xiegu.com/) [X6100](https://www.xiegu.com/)“, „XIEGU [X6200](https://www.xiegu.com/)“ oder als Fallback **IC-7100**; Steuerung über **[rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html)** am ESP (`[rigctl](https://hamlib.sourceforge.net/html/rigctl.1.html) -m 2 -r <ESP-IP>:4532`).

## Yaesu

- **Protokoll:** [Yaesu CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) (ASCII-Befehle).
- **Baud:** 38400 (FT-991A, FT-910, FT-DX-Serie, FT-891); **4800** für FT-897/FT-857 – im Funkmenü **[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) RATE** anpassen, falls nötig.
- **[FT8](https://de.wikipedia.org/wiki/FT8_(Amateurfunk))/Digital:** Modus **[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)** oder **DATA-[USB](https://de.wikipedia.org/wiki/Universal_Serial_Bus)** / **DIGU** je nach Gerät; [PTT](https://de.wikipedia.org/wiki/Sende-Empfang-Umschaltung) über [CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver) (`T 3` für Daten) wird vom Panel unterstützt.
- **Audio:** [USB-Audio](https://de.wikipedia.org/wiki/Universal_Serial_Bus-Audioklasse) oder LINE OUT / LINE IN / PACKET – physikalisch an **[I2S](https://de.wikipedia.org/wiki/I%C2%B2S)-Bridge** ([PCM5102](https://www.ti.com/product/PCM5102) + [INMP441](https://www.invensense.com/products/digital/inmp441/) oder Line-Level) anschließen, siehe README.

## [Hamlib](https://en.wikipedia.org/wiki/Hamlib) / [WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html) am PC

| Gerät | Empfohlenes [Hamlib](https://en.wikipedia.org/wiki/Hamlib)-Modell (Referenz) |
|-------|--------------------------------------|
| [Xiegu](https://www.xiegu.com/) G90 | [Xiegu](https://www.xiegu.com/) G90 / IC-7100 |
| [X6100](https://www.xiegu.com/) / [X6200](https://www.xiegu.com/) | [Xiegu](https://www.xiegu.com/) [X6100](https://www.xiegu.com/) / [X6200](https://www.xiegu.com/) |
| FT-991A | FT-991 / FT-991A |
| FT-DX10 / 101D / 101MP | Entsprechendes FT-DX-Modell |
| FT-891 / 897 / 857 | Gleicher Modellname in [Hamlib](https://en.wikipedia.org/wiki/Hamlib) |

Steuerung über das Panel:

```bash
rigctl -m 2 -r 192.168.4.1:4532 f
rigctl -m 2 -r 192.168.4.1:4532 m USB
```

(`-m 2` = [Hamlib](https://en.wikipedia.org/wiki/Hamlib)-Netzwerk-Rig; das Panel antwortet als [rigctld](https://hamlib.sourceforge.net/html/rigctld.1.html).)

## Audio über [WiFi](https://de.wikipedia.org/wiki/WLAN) ([FT8](https://de.wikipedia.org/wiki/FT8_(Amateurfunk)))

- Sample Rate **48000 Hz** (Standard in der Konfiguration).
- [UDP](https://de.wikipedia.org/wiki/User_Datagram_Protocol) **4533** (Funk → PC) und **4534** (PC → Funk), oder Browser unter `/audio`.
- Skripte: `scripts/ft8_remote.py`, `scripts/ft8_linux_pa.sh`, `scripts/ft8_windows_setup.ps1`.

## Hardware-Hinweis

[CAT](https://en.wikipedia.org/wiki/Computer_Aided_Transceiver)-Leitungen des Funkgeräts sind oft **5 V [TTL](https://de.wikipedia.org/wiki/Transistor-Transistor-Logik)** – zwischen [ESP32](https://de.wikipedia.org/wiki/ESP32) (3,3 V) und Radio einen **Bidirectional-[Level-Shifter](https://de.wikipedia.org/wiki/Pegelwandler)** verwenden (kein [MAX3232](https://de.wikipedia.org/wiki/MAX3232), sofern kein echtes [RS-232](https://de.wikipedia.org/wiki/RS-232) ±12 V).
