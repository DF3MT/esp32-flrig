# Unterstützte Funkgeräte (CAT + Audio)

Das ESP32-Panel spricht **direkt per UART** mit dem Funkgerät (3,3 V TTL über Pegelwandler). In der Web-Konfiguration (`http://<ESP-IP>/`) wählen Sie das Modell – **Baudrate, Protokoll und CI-V-Adresse** werden automatisch gesetzt.

## Übersicht

| Profil-ID | Gerät | CAT-Protokoll | Baud | CI-V Addr |
|-----------|-------|---------------|------|-----------|
| `XIEGU_G90` | Xiegu G90 | ICOM CI-V | 19200 | A4 |
| `XIEGU_X6100` | Xiegu X6100 | ICOM CI-V | 19200 | A4 |
| `XIEGU_X6200` | Xiegu X6200 | ICOM CI-V | 19200 | A4 |
| `YAESU_FT991A` | Yaesu FT-991A | Yaesu CAT | 38400 | — |
| `YAESU_FT910` | Yaesu FT-910 | Yaesu CAT | 38400 | — |
| `YAESU_FTDX10` | Yaesu FT-DX10 | Yaesu CAT | 38400 | — |
| `YAESU_FTDX101D` | Yaesu FT-DX101D | Yaesu CAT | 38400 | — |
| `YAESU_FTDX101MP` | Yaesu FT-DX101MP | Yaesu CAT | 38400 | — |
| `YAESU_FT891` | Yaesu FT-891 | Yaesu CAT | 38400 | — |
| `YAESU_FT897` | Yaesu FT-897 | Yaesu CAT | 4800* | — |
| `YAESU_FT857` | Yaesu FT-857 | Yaesu CAT | 4800 | — |

\* FT-897: Menü-CAT-Rate prüfen (oft 4800, manche 38400).

## Xiegu (G90, X6100, X6200)

- **Protokoll:** CI-V-kompatibel (wie ICOM), **nicht** Yaesu-CAT.
- **Einstellungen:** 19200 bps, Radio-Adresse **0xA4** (wird vom Profil gesetzt).
- **Anschluss G90:** CAT nur an der **Display-Einheit** (3,5 mm / IO-Port), nicht am Basisgerät.
- **X6100/X6200:** USB-C DEV, Serial-B für CAT; USB-Audio für Sound.
- **WSJT-X / flrig:** Modell z. B. „Xiegu X6100“, „XIEGU X6200“ oder als Fallback **IC-7100**; Steuerung über **rigctld** am ESP (`rigctl -m 2 -r <ESP-IP>:4532`).

## Yaesu

- **Protokoll:** Yaesu CAT (ASCII-Befehle).
- **Baud:** 38400 (FT-991A, FT-910, FT-DX-Serie, FT-891); **4800** für FT-897/FT-857 – im Funkmenü **CAT RATE** anpassen, falls nötig.
- **FT8/Digital:** Modus **USB** oder **DATA-USB** / **DIGU** je nach Gerät; PTT über CAT (`T 3` für Daten) wird vom Panel unterstützt.
- **Audio:** USB-Audio oder LINE OUT / LINE IN / PACKET – physikalisch an **I2S-Bridge** (PCM5102 + INMP441 oder Line-Level) anschließen, siehe README.

## Hamlib / WSJT-X am PC

| Gerät | Empfohlenes Hamlib-Modell (Referenz) |
|-------|--------------------------------------|
| Xiegu G90 | Xiegu G90 / IC-7100 |
| X6100 / X6200 | Xiegu X6100 / X6200 |
| FT-991A | FT-991 / FT-991A |
| FT-DX10 / 101D / 101MP | Entsprechendes FT-DX-Modell |
| FT-891 / 897 / 857 | Gleicher Modellname in Hamlib |

Steuerung über das Panel:

```bash
rigctl -m 2 -r 192.168.4.1:4532 f
rigctl -m 2 -r 192.168.4.1:4532 m USB
```

(`-m 2` = Hamlib-Netzwerk-Rig; das Panel antwortet als rigctld.)

## Audio über WiFi (FT8)

- Sample Rate **48000 Hz** (Standard in der Konfiguration).
- UDP **4533** (Funk → PC) und **4534** (PC → Funk), oder Browser unter `/audio`.
- Skripte: `scripts/ft8_remote.py`, `scripts/ft8_linux_pa.sh`, `scripts/ft8_windows_setup.ps1`.

## Hardware-Hinweis

CAT-Leitungen des Funkgeräts sind oft **5 V TTL** – zwischen ESP32 (3,3 V) und Radio einen **Bidirectional-Level-Shifter** verwenden (kein MAX3232, sofern kein echtes RS-232 ±12 V).
