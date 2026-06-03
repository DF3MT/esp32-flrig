#include "rotctld_server.h"
#include "hamlib_tcp_server.h"

static bool parseTwoFloats(const String& cmd, float& a, float& b) {
    int sp = cmd.indexOf(' ');
    if (sp < 0) return false;
    String rest = cmd.substring(sp + 1);
    rest.trim();
    int sp2 = rest.indexOf(' ');
    if (sp2 < 0) {
        a = rest.toFloat();
        b = 0.0f;
        return true;
    }
    a = rest.substring(0, sp2).toFloat();
    b = rest.substring(sp2 + 1).toFloat();
    return true;
}

static bool parseMove(const String& cmd, int& dir, int& speed) {
    int sp = cmd.indexOf(' ');
    if (sp < 0) return false;
    String rest = cmd.substring(sp + 1);
    rest.trim();
    int sp2 = rest.indexOf(' ');
    dir = rest.substring(0, sp2 > 0 ? sp2 : rest.length()).toInt();
    if (sp2 > 0)
        speed = rest.substring(sp2 + 1).toInt();
    else
        speed = 50;
    return true;
}

String dispatchRotctl(const String& line, RotorController& rotor, const AppConfig* cfg) {
    String cmd = line;
    cmd.trim();

    if (cmd.length() == 0) return hamlibRprtOk();

    // Extended Response Protocol: leading '+' — strip for dispatch
    if (cmd.charAt(0) == '+' || cmd.charAt(0) == ';' || cmd.charAt(0) == '|')
        cmd = cmd.substring(1);

    if (cmd == "q" || cmd == "quit") return hamlibRprtOk();

    if (cmd == "p" || cmd.startsWith("get_pos")) {
        float az, el;
        rotor.getPosition(az, el);
        return String(az, 2) + "\n" + String(el, 2) + "\n" + hamlibRprtOk();
    }

    if (cmd.startsWith("P ") || cmd.startsWith("set_pos")) {
        float az, el;
        if (parseTwoFloats(cmd, az, el) && rotor.setPosition(az, el))
            return hamlibRprtOk();
        return hamlibRprtErr(-1);
    }

    if (cmd.startsWith("M ") || cmd.startsWith("move")) {
        int dir, speed;
        if (parseMove(cmd, dir, speed) && rotor.move(dir, speed))
            return hamlibRprtOk();
        return hamlibRprtErr(-1);
    }

    if (cmd == "S" || cmd == "stop") {
        rotor.stop();
        return hamlibRprtOk();
    }

    if (cmd == "K" || cmd == "park") {
        rotor.park();
        return hamlibRprtOk();
    }

    if (cmd == "_" || cmd.startsWith("get_info")) {
        String info = "ESP32 FLRIG Rotor (GPIO OC)";
        if (cfg && !cfg->rotorEnabled)
            info += " [disabled]";
        return info + "\n" + hamlibRprtOk();
    }

    if (cmd == "1" || cmd == "\\dump_caps" || cmd == "\\dump_state")
        return String("0\n") + hamlibRprtOk();

    if (cmd.startsWith("R ") || cmd.startsWith("reset"))
        return hamlibRprtOk();

    if (cmd.startsWith("C ") || cmd.startsWith("set_conf"))
        return hamlibRprtOk();

    if (cmd.startsWith("w ") || cmd.startsWith("send_cmd"))
        return hamlibRprtErr(-4);

    Serial.printf("[rotctld] unknown: %s\n", cmd.c_str());
    return hamlibRprtErr(-4);
}
