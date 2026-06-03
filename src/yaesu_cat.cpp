#include "yaesu_cat.h"

bool YaesuCat::begin(Stream& serial, uint32_t baud) {
    (void)baud;
    _serial = &serial;
    _rxBuf = "";
    return true;
}

bool YaesuCat::sendCommand(const char* cmd) {
    if (!_serial) return false;
    _serial->print(cmd);
    if (cmd[strlen(cmd) - 1] != ';') _serial->print(';');
    _serial->flush();
    return true;
}

bool YaesuCat::queryCommand(const char* cmd, String& response, uint32_t timeoutMs) {
    _rxBuf = "";
    sendCommand(cmd);
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        while (_serial->available()) {
            char c = _serial->read();
            _rxBuf += c;
            if (c == ';') {
                response = _rxBuf;
                return true;
            }
        }
        delay(1);
    }
    return false;
}

bool YaesuCat::setFrequency(uint64_t hz) {
    char buf[32];
    snprintf(buf, sizeof(buf), "FA%011llu", (unsigned long long)hz);
    return sendCommand(buf);
}

bool YaesuCat::getFrequency(uint64_t& hz) {
    String resp;
    if (!queryCommand("FA", resp)) return false;
    // FA00014074000;
    if (resp.length() >= 14) {
        hz = strtoull(resp.substring(2, 13).c_str(), nullptr, 10);
        return true;
    }
    return false;
}

char YaesuCat::yaesuModeFromStr(const char* mode) {
    if (!mode) return '2';
    if (strcasecmp(mode, "LSB") == 0) return '1';
    if (strcasecmp(mode, "USB") == 0) return '2';
    if (strcasecmp(mode, "CW")  == 0) return '3';
    if (strcasecmp(mode, "FM")  == 0) return '4';
    if (strcasecmp(mode, "AM")  == 0) return '5';
    if (strcasecmp(mode, "RTTY") == 0) return '6';
    if (strcasecmp(mode, "CW-R") == 0 || strcasecmp(mode, "CWR") == 0) return '7';
    if (strcasecmp(mode, "DATA-LSB") == 0 || strcasecmp(mode, "DIGL") == 0) return '8';
    if (strcasecmp(mode, "DATA") == 0 || strcasecmp(mode, "DIG") == 0 ||
        strcasecmp(mode, "DIGI") == 0) return '8';
    // FT-991/910/DX: DATA-USB, PKTUSB, FT8 (WSJT-X)
    if (strcasecmp(mode, "DATA-USB") == 0 || strcasecmp(mode, "USB-D") == 0 ||
        strcasecmp(mode, "PKTUSB") == 0 || strcasecmp(mode, "PKT") == 0 ||
        strcasecmp(mode, "DIGU") == 0 || strcasecmp(mode, "FT8") == 0 ||
        strcasecmp(mode, "FM-D") == 0) return '9';
    if (strcasecmp(mode, "AM-D") == 0) return 'A';
    return '2';
}

void YaesuCat::strFromYaesuMode(char ymode, char* out, size_t len) {
    const char* m = "USB";
    switch (ymode) {
        case '1': m = "LSB"; break;
        case '2': m = "USB"; break;
        case '3': m = "CW";  break;
        case '4': m = "FM";  break;
        case '5': m = "AM";  break;
        case '6': m = "RTTY"; break;
        case '7': m = "CW-R"; break;
        case '8': m = "DATA"; break;
        case '9': m = "DATA-USB"; break;
        case 'A': m = "AM-D"; break;
        default: break;
    }
    strncpy(out, m, len - 1);
    out[len - 1] = '\0';
}

bool YaesuCat::setMode(const char* mode) {
    char buf[8];
    snprintf(buf, sizeof(buf), "MD0%c", yaesuModeFromStr(mode));
    return sendCommand(buf);
}

bool YaesuCat::getMode(char* mode, size_t len) {
    String resp;
    if (!queryCommand("MD0", resp)) return false;
    // MD02;
    if (resp.length() >= 4) {
        strFromYaesuMode(resp.charAt(3), mode, len);
        return true;
    }
    return false;
}

bool YaesuCat::setPtt(bool on) {
    return sendCommand(on ? "TX1" : "TX0");
}

bool YaesuCat::setLevel(const char* level, float value) {
    if (strcasecmp(level, "RFPOWER") == 0 || strcasecmp(level, "RF") == 0) {
        char buf[16];
        int pc = static_cast<int>(value * 100.0f);
        snprintf(buf, sizeof(buf), "PC%03d", pc);
        return sendCommand(buf);
    }
    if (strcasecmp(level, "AF") == 0) {
        char buf[16];
        int vol = static_cast<int>(value * 255.0f);
        snprintf(buf, sizeof(buf), "AG%04d", vol);
        return sendCommand(buf);
    }
    if (strcasecmp(level, "SQL") == 0 || strcasecmp(level, "SQUELCH") == 0) {
        char buf[16];
        int sq = static_cast<int>(value * 255.0f);
        snprintf(buf, sizeof(buf), "SQ%04d", sq);
        return sendCommand(buf);
    }
    return false;
}

bool YaesuCat::sendRaw(const uint8_t* data, size_t len) {
    if (!_serial) return false;
    _serial->write(data, len);
    _serial->flush();
    return true;
}

void YaesuCat::poll() {
    // Yaesu transceive: process unsolicited updates if needed
    while (_serial && _serial->available()) {
        _serial->read();
    }
}
