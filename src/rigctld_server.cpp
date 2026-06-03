#include "rigctld_server.h"

String rigctlOk()  { return "RPRT 0\n"; }
String rigctlErr(int code) {
    return "RPRT " + String(code) + "\n";
}

bool RigctldServer::begin(uint16_t port) {
    _port = port;
    _server = new WiFiServer(port);
    _server->begin();
    _server->setNoDelay(true);
    Serial.printf("[rigctld] listening on port %u\n", port);
    return true;
}

void RigctldServer::loop() {
    if (!_server) return;

    if (!_client || !_client.connected()) {
        _client = _server->available();
        if (_client) {
            _client.setNoDelay(true);
            _buffer = "";
            Serial.println("[rigctld] client connected");
        }
        return;
    }

    while (_client.available()) {
        char c = _client.read();
        if (c == '\n' || c == '\r') {
            if (_buffer.length() > 0) {
                String resp = processLine(_buffer);
                _client.print(resp);
                _buffer = "";
            }
        } else {
            _buffer += c;
        }
    }
}

String RigctldServer::processLine(const String& line) {
    if (_handler) return _handler(line);

    String cmd = line;
    cmd.trim();
    if (cmd == "q" || cmd == "quit") {
        _client.stop();
        return rigctlOk();
    }
    return rigctlErr(-1);
}

static bool parseSetFreq(const String& cmd, uint64_t& hz) {
    int sp = cmd.indexOf(' ');
    if (sp < 0) return false;
    hz = strtoull(cmd.substring(sp + 1).c_str(), nullptr, 10);
    return true;
}

static bool parseSetMode(const String& cmd, String& mode) {
    int sp = cmd.indexOf(' ');
    if (sp < 0) return false;
    mode = cmd.substring(sp + 1);
    int sp2 = mode.indexOf(' ');
    if (sp2 > 0) mode = mode.substring(0, sp2);
    return true;
}

String dispatchRigctl(const String& line, CatController& cat, RadioState& state) {
    String cmd = line;
    cmd.trim();

    // quit
    if (cmd == "q" || cmd == "quit") return rigctlOk();

    // get frequency
    if (cmd == "f" || cmd.startsWith("get_freq")) {
        uint64_t hz = 0;
        cat.getFrequency(hz);
        state.freqHz = hz;
        return String(hz) + "\n" + rigctlOk();
    }

    // set frequency
    if (cmd.startsWith("F ") || cmd.startsWith("set_freq")) {
        uint64_t hz = 0;
        if (parseSetFreq(cmd, hz) && cat.setFrequency(hz))
            return rigctlOk();
        return rigctlErr(-1);
    }

    // get mode
    if (cmd == "m" || cmd.startsWith("get_mode")) {
        char mode[16] = {};
        cat.getMode(mode, sizeof(mode));
        return String(mode) + "\n0\n" + rigctlOk();
    }

    // set mode
    if (cmd.startsWith("M ") || cmd.startsWith("set_mode")) {
        String mode;
        if (parseSetMode(cmd, mode) && cat.setMode(mode.c_str()))
            return rigctlOk();
        return rigctlErr(-1);
    }

    // PTT get/set
    if (cmd == "t" || cmd.startsWith("get_ptt"))
        return String(state.ptt ? 1 : 0) + "\n" + rigctlOk();

    if (cmd.startsWith("T ") || cmd.startsWith("set_ptt")) {
        int sp = cmd.indexOf(' ');
        bool on = cmd.substring(sp + 1).toInt() != 0;
        if (cat.setPtt(on)) return rigctlOk();
        return rigctlErr(-1);
    }

    // levels: L RFPOWER 0.5 / l RFPOWER
    if (cmd.startsWith("L ") || cmd.startsWith("set_level")) {
        int sp1 = cmd.indexOf(' ');
        int sp2 = cmd.indexOf(' ', sp1 + 1);
        if (sp2 > 0) {
            String level = cmd.substring(sp1 + 1, sp2);
            float val = cmd.substring(sp2 + 1).toFloat();
            if (cat.setLevel(level.c_str(), val)) return rigctlOk();
        }
        return rigctlErr(-1);
    }

    if (cmd.startsWith("l ") || cmd.startsWith("get_level")) {
        int sp = cmd.indexOf(' ');
        String level = cmd.substring(sp + 1);
        level.trim();
        float val = 0.5f;
        if (level.equalsIgnoreCase("AF")) val = state.afLevel;
        else if (level.equalsIgnoreCase("RFPOWER") || level.equalsIgnoreCase("RF"))
            val = state.rfPower;
        return String(val, 4) + "\n" + rigctlOk();
    }

    // VFO
    if (cmd == "v" || cmd.startsWith("get_vfo"))
        return String("VFOA\n") + rigctlOk();

    if (cmd.startsWith("V ") || cmd.startsWith("set_vfo"))
        return rigctlOk();

    // raw hex passthrough (hamlib 'w')
    if (cmd.startsWith("w ") || cmd.startsWith("send_raw")) {
        int sp = cmd.indexOf(' ');
        if (cat.sendRawHex(cmd.substring(sp + 1).c_str()))
            return rigctlOk();
        return rigctlErr(-1);
    }

    // hamlib handshake commands
    if (cmd == "\\get_info")
        return String("ESP32 CAT Remote Panel\n") + rigctlOk();

    if (cmd == "\\chk_vfo")
        return String("0\n") + rigctlOk();

    if (cmd == "\\dump_state" || cmd == "1" || cmd == "\\dump_caps")
        return String("0\n") + rigctlOk();

    Serial.printf("[rigctld] unknown cmd: %s\n", cmd.c_str());
    return rigctlErr(-4);  // RIG_EINVAL
}
