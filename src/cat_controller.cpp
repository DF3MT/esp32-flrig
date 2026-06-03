#include "cat_controller.h"
#include "icom_civ.h"
#include "yaesu_cat.h"

static IcomCiv  s_icom;
static YaesuCat s_yaesu;

bool CatController::begin(Stream& serial, RadioVendor vendor, uint32_t baud, uint8_t icomAddr) {
    _serial = &serial;
    _vendor = vendor;
    _icomAddr = icomAddr;
    s_icom.setRadioAddress(icomAddr);

    if (vendor == RadioVendor::ICOM) {
        _proto = &s_icom;
    } else {
        _proto = &s_yaesu;
    }

    if (!_proto->begin(*_serial, baud)) return false;

    _state.connected = true;
    getFrequency(_state.freqHz);
    getMode(_state.mode, sizeof(_state.mode));
    return true;
}

void CatController::setVendor(RadioVendor v) {
    if (v == _vendor) return;
    _vendor = v;
    _proto = (v == RadioVendor::ICOM) ? static_cast<CatProtocol*>(&s_icom)
                                       : static_cast<CatProtocol*>(&s_yaesu);
}

void CatController::poll() {
    if (_proto) _proto->poll();
}

bool CatController::setFrequency(uint64_t hz) {
    if (!_proto) return false;
    if (_proto->setFrequency(hz)) {
        _state.freqHz = hz;
        if (_callback) _callback(_state);
        return true;
    }
    return false;
}

bool CatController::getFrequency(uint64_t& hz) {
    if (!_proto) return false;
    if (_proto->getFrequency(hz)) {
        _state.freqHz = hz;
        return true;
    }
    return false;
}

bool CatController::setMode(const char* mode) {
    if (!_proto) return false;
    if (_proto->setMode(mode)) {
        strncpy(_state.mode, mode, sizeof(_state.mode) - 1);
        if (_callback) _callback(_state);
        return true;
    }
    return false;
}

bool CatController::getMode(char* mode, size_t len) {
    return _proto ? _proto->getMode(mode, len) : false;
}

bool CatController::setPtt(bool on) {
    if (!_proto) return false;
    if (_proto->setPtt(on)) {
        _state.ptt = on;
        if (_callback) _callback(_state);
        return true;
    }
    return false;
}

bool CatController::setLevel(const char* level, float value) {
    if (!_proto) return false;
    if (_proto->setLevel(level, value)) {
        if (strcasecmp(level, "AF") == 0) _state.afLevel = value;
        if (strcasecmp(level, "RFPOWER") == 0 || strcasecmp(level, "RF") == 0)
            _state.rfPower = value;
        if (_callback) _callback(_state);
        return true;
    }
    return false;
}

bool CatController::sendRawHex(const char* hex) {
    if (!_proto || !hex) return false;
    uint8_t buf[64];
    size_t len = 0;
    for (const char* p = hex; *p && len < sizeof(buf); p += 2) {
        while (*p == ' ') p++;
        if (!*p) break;
        char byteStr[3] = {p[0], p[1], 0};
        buf[len++] = static_cast<uint8_t>(strtoul(byteStr, nullptr, 16));
    }
    return _proto->sendRaw(buf, len);
}
