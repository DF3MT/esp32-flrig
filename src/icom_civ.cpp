#include "icom_civ.h"

IcomCiv::IcomCiv(uint8_t radioAddr, uint8_t ctrlAddr)
    : _radioAddr(radioAddr), _ctrlAddr(ctrlAddr) {}

bool IcomCiv::begin(HardwareSerial& serial, uint32_t baud) {
    _serial = &serial;
    _serial->begin(baud, SERIAL_8N1, CAT_RX_PIN, CAT_TX_PIN);
    _rxLen = 0;
    return true;
}

bool IcomCiv::sendFrame(const uint8_t* data, size_t len) {
    if (!_serial) return false;
    _serial->write(0xFE);
    _serial->write(0xFE);
    _serial->write(_radioAddr);
    _serial->write(_ctrlAddr);
    _serial->write(data, len);
    _serial->write(0xFD);
    _serial->flush();
    return true;
}

bool IcomCiv::transact(const uint8_t* cmd, size_t cmdLen,
                         uint8_t* resp, size_t& respLen, uint32_t timeoutMs) {
    _rxLen = 0;
    sendFrame(cmd, cmdLen);

    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        while (_serial->available()) {
            uint8_t b = _serial->read();
            if (_rxLen < sizeof(_rxBuf)) _rxBuf[_rxLen++] = b;
            if (b == 0xFD && _rxLen >= 6) {
                respLen = _rxLen;
                memcpy(resp, _rxBuf, respLen);
                _rxLen = 0;
                return true;
            }
        }
        delay(1);
    }
    return false;
}

void IcomCiv::freqToBcd(uint64_t hz, uint8_t out[5]) {
    // 10 Hz units, little-endian BCD (IC-705/7300 style)
    uint64_t units = hz / 10;
    for (int i = 0; i < 5; i++) {
        uint8_t lo = units % 10;
        units /= 10;
        uint8_t hi = units % 10;
        units /= 10;
        out[i] = (hi << 4) | lo;
    }
}

uint64_t IcomCiv::bcdToFreq(const uint8_t bcd[5]) {
    uint64_t units = 0;
    for (int i = 4; i >= 0; i--) {
        units = units * 100 + ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);
    }
    return units * 10;
}

uint8_t IcomCiv::modeToIcom(const char* mode) {
    if (!mode) return 0x01;
    if (strcasecmp(mode, "LSB") == 0) return 0x00;
    if (strcasecmp(mode, "USB") == 0) return 0x01;
    if (strcasecmp(mode, "AM")  == 0) return 0x02;
    if (strcasecmp(mode, "CW")  == 0) return 0x03;
    if (strcasecmp(mode, "RTTY") == 0) return 0x04;
    if (strcasecmp(mode, "FM")  == 0) return 0x05;
    if (strcasecmp(mode, "DIG") == 0 || strcasecmp(mode, "DATA") == 0 ||
        strcasecmp(mode, "DIGI") == 0) return 0x0A;
    if (strcasecmp(mode, "USB-D") == 0 || strcasecmp(mode, "DATA-USB") == 0 ||
        strcasecmp(mode, "PKTUSB") == 0 || strcasecmp(mode, "DIGU") == 0 ||
        strcasecmp(mode, "FT8") == 0) return 0x01;
    if (strcasecmp(mode, "DATA-LSB") == 0 || strcasecmp(mode, "DIGL") == 0) return 0x00;
    return 0x01;
}

void IcomCiv::icomToMode(uint8_t icomMode, uint8_t /*filter*/, char* out, size_t len) {
    const char* m = "USB";
    switch (icomMode) {
        case 0x00: m = "LSB"; break;
        case 0x01: m = "USB"; break;
        case 0x02: m = "AM";  break;
        case 0x03: m = "CW";  break;
        case 0x04: m = "RTTY"; break;
        case 0x05: m = "FM";  break;
        case 0x0A: m = "DATA"; break;
        default: break;
    }
    strncpy(out, m, len - 1);
    out[len - 1] = '\0';
}

bool IcomCiv::setFrequency(uint64_t hz) {
    uint8_t bcd[5];
    freqToBcd(hz, bcd);
    uint8_t cmd[] = {0x05, bcd[0], bcd[1], bcd[2], bcd[3], bcd[4]};
    uint8_t resp[32];
    size_t respLen = 0;
    return transact(cmd, sizeof(cmd), resp, respLen, 300);
}

bool IcomCiv::getFrequency(uint64_t& hz) {
    uint8_t cmd[] = {0x03};
    uint8_t resp[32];
    size_t respLen = 0;
    if (!transact(cmd, sizeof(cmd), resp, respLen, 500)) return false;
    // FE FE E0 94 03 [bcd x5] FD  or  FE FE 94 E0 FB FD + FE FE 94 E0 03 ...
    for (size_t i = 0; i + 8 < respLen; i++) {
        if (resp[i] == 0x03 && i + 6 < respLen) {
            hz = bcdToFreq(&resp[i + 1]);
            return true;
        }
    }
    return false;
}

bool IcomCiv::setMode(const char* mode) {
    uint8_t cmd[] = {0x06, modeToIcom(mode), 0x01};  // filter width 1
    uint8_t resp[32];
    size_t respLen = 0;
    return transact(cmd, sizeof(cmd), resp, respLen, 300);
}

bool IcomCiv::getMode(char* mode, size_t len) {
    uint8_t cmd[] = {0x04};
    uint8_t resp[32];
    size_t respLen = 0;
    if (!transact(cmd, sizeof(cmd), resp, respLen, 500)) return false;
    for (size_t i = 0; i + 3 < respLen; i++) {
        if (resp[i] == 0x04) {
            icomToMode(resp[i + 1], resp[i + 2], mode, len);
            return true;
        }
    }
    return false;
}

bool IcomCiv::setPtt(bool on) {
    uint8_t cmd[] = {0x1C, 0x00, static_cast<uint8_t>(on ? 0x01 : 0x00)};
    uint8_t resp[32];
    size_t respLen = 0;
    return transact(cmd, sizeof(cmd), resp, respLen, 300);
}

bool IcomCiv::setLevel(const char* level, float value) {
    uint16_t v = static_cast<uint16_t>(value * 255.0f);
    if (strcasecmp(level, "RFPOWER") == 0 || strcasecmp(level, "RF") == 0) {
        uint8_t cmd[] = {0x14, 0x0A, static_cast<uint8_t>(v >> 8), static_cast<uint8_t>(v & 0xFF)};
        uint8_t resp[32]; size_t rl = 0;
        return transact(cmd, sizeof(cmd), resp, rl, 300);
    }
    if (strcasecmp(level, "AF") == 0) {
        uint8_t cmd[] = {0x14, 0x06, static_cast<uint8_t>(v >> 8), static_cast<uint8_t>(v & 0xFF)};
        uint8_t resp[32]; size_t rl = 0;
        return transact(cmd, sizeof(cmd), resp, rl, 300);
    }
    return false;
}

bool IcomCiv::sendRaw(const uint8_t* data, size_t len) {
    if (!_serial) return false;
    _serial->write(data, len);
    _serial->flush();
    return true;
}

void IcomCiv::parseIncoming() {
    while (_serial && _serial->available()) {
        uint8_t b = _serial->read();
        if (_rxLen < sizeof(_rxBuf)) _rxBuf[_rxLen++] = b;
        if (b == 0xFD) {
            _rxLen = 0;
        }
    }
}

void IcomCiv::poll() {
    parseIncoming();
}
