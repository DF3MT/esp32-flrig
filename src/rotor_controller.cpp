#include "rotor_controller.h"

void RotorController::setOc(uint8_t pin, bool active) {
    if (pin == 255) return;
    // Open collector: LOW = Treiber aktiv (Relais ein), HIGH = aus
    digitalWrite(pin, active ? LOW : HIGH);
}

void RotorController::applyOutputs() {
    if (!_enabled) return;
    bool left  = (_state.moving & RotorDir::LEFT) != 0;
    bool right = (_state.moving & RotorDir::RIGHT) != 0;
    // Azimut: CCW = links, CW = rechts
    setOc(_ocCcw, left);
    setOc(_ocCw, right);
}

bool RotorController::begin(const AppConfig& cfg) {
    _enabled = cfg.rotorEnabled;
    _btnCcw = cfg.rotorBtnCcw;
    _btnCw  = cfg.rotorBtnCw;
    _ocCcw  = cfg.rotorOcCcw;
    _ocCw   = cfg.rotorOcCw;
    _debounceMs = cfg.rotorDebounceMs;
    _state.speed = cfg.rotorSpeed;

    if (!_enabled) {
        Serial.println("[rotor] disabled");
        return true;
    }

    pinMode(_btnCcw, INPUT_PULLUP);
    pinMode(_btnCw, INPUT_PULLUP);
    pinMode(_ocCcw, OUTPUT_OPEN_DRAIN);
    pinMode(_ocCw, OUTPUT_OPEN_DRAIN);
    setOc(_ocCcw, false);
    setOc(_ocCw, false);

    _btnCcwLast = digitalRead(_btnCcw) == HIGH;
    _btnCwLast  = digitalRead(_btnCw) == HIGH;

    Serial.printf("[rotor] btn CCW=%u CW=%u OC CCW=%u CW=%u (open drain)\n",
                  _btnCcw, _btnCw, _ocCcw, _ocCw);
    return true;
}

void RotorController::loop() {
    if (!_enabled) return;

    uint32_t now = millis();
    if (now - _lastBtnMs < _debounceMs) return;

    bool ccw = digitalRead(_btnCcw) == LOW;
    bool cw  = digitalRead(_btnCw) == LOW;

    if (ccw != _btnCcwLast || cw != _btnCwLast) {
        _lastBtnMs = now;
        _btnCcwLast = ccw;
        _btnCwLast = cw;

        if (ccw && !cw)
            move(RotorDir::LEFT, _state.speed);
        else if (cw && !ccw)
            move(RotorDir::RIGHT, _state.speed);
        else if (!ccw && !cw)
            stop();
        // beide gedrückt → stop (Sicherheit)
        else
            stop();
    }
}

bool RotorController::move(int direction, int speed) {
    if (!_enabled) return false;
    if (speed < 1) speed = 1;
    if (speed > 100) speed = 100;
    _state.speed = speed;
    _state.moving = direction & (RotorDir::LEFT | RotorDir::RIGHT | RotorDir::UP | RotorDir::DOWN);
    if (_state.moving == 0) {
        stop();
        return true;
    }
    applyOutputs();
    return true;
}

void RotorController::stop() {
    _state.moving = 0;
    applyOutputs();
}

bool RotorController::park() {
    stop();
    return true;
}

bool RotorController::setPosition(float az, float el) {
    stop();
    _state.azimuth = az;
    _state.elevation = el;
    return true;
}

void RotorController::getPosition(float& az, float& el) const {
    az = _state.azimuth;
    el = _state.elevation;
}
