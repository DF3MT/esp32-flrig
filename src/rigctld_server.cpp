#include "rigctld_server.h"
#include "radio_profiles.h"

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

String dispatchRigctl(const String& line, CatController& cat, RadioState& state,
                      const AppConfig* cfg, const RadioChannelConfig* channel) {
    String cmd = line;
    cmd.trim();

    if (cmd == "q" || cmd == "quit") return rigctlOk();

    if (cmd == "f" || cmd.startsWith("get_freq")) {
        uint64_t hz = 0;
        cat.getFrequency(hz);
        state.freqHz = hz;
        return String(hz) + "\n" + rigctlOk();
    }

    if (cmd.startsWith("F ") || cmd.startsWith("set_freq")) {
        uint64_t hz = 0;
        if (parseSetFreq(cmd, hz) && cat.setFrequency(hz))
            return rigctlOk();
        return rigctlErr(-1);
    }

    if (cmd == "m" || cmd.startsWith("get_mode")) {
        char mode[16] = {};
        cat.getMode(mode, sizeof(mode));
        return String(mode) + "\n0\n" + rigctlOk();
    }

    if (cmd.startsWith("M ") || cmd.startsWith("set_mode")) {
        String mode;
        if (parseSetMode(cmd, mode) && cat.setMode(mode.c_str()))
            return rigctlOk();
        return rigctlErr(-1);
    }

    if (cmd == "t" || cmd.startsWith("get_ptt"))
        return String(state.ptt ? 1 : 0) + "\n" + rigctlOk();

    if (cmd.startsWith("T ") || cmd.startsWith("set_ptt")) {
        int sp = cmd.indexOf(' ');
        int v = cmd.substring(sp + 1).toInt();
        bool on = (v == 1 || v == 3);
        if (cat.setPtt(on)) return rigctlOk();
        return rigctlErr(-1);
    }

    if (cmd == "s" || cmd.startsWith("get_split_vfo") || cmd == "\\get_split_vfo")
        return String("0\nVFOA\n") + rigctlOk();

    if (cmd.startsWith("S ") || cmd.startsWith("set_split_vfo") || cmd.startsWith("\\set_split_vfo"))
        return rigctlOk();

    if (cmd.startsWith("I ") || cmd.startsWith("set_split_freq") || cmd.startsWith("\\set_split_freq"))
        return rigctlOk();

    if (cmd == "i" || cmd.startsWith("get_split_freq") || cmd.startsWith("\\get_split_freq")) {
        uint64_t hz = state.freqHz;
        cat.getFrequency(hz);
        state.freqHz = hz;
        return String(hz) + "\n" + rigctlOk();
    }

    if (cmd.startsWith("K ") || cmd.startsWith("set_split_freq_mode") || cmd.startsWith("X ") ||
        cmd.startsWith("set_split_mode") || cmd.startsWith("\\set_split_mode"))
        return rigctlOk();

    if (cmd.startsWith("x ") || cmd.startsWith("get_split_mode") || cmd.startsWith("\\get_split_mode"))
        return String("USB\n0\n") + rigctlOk();

    if (cmd.startsWith("U ") || cmd.startsWith("set_func") || cmd.startsWith("\\set_func"))
        return rigctlOk();
    if (cmd.startsWith("u ") || cmd.startsWith("get_func") || cmd.startsWith("\\get_func"))
        return String("0\n") + rigctlOk();

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

    if (cmd == "v" || cmd.startsWith("get_vfo"))
        return String("VFOA\n") + rigctlOk();

    if (cmd.startsWith("V ") || cmd.startsWith("set_vfo"))
        return rigctlOk();

    if (cmd.startsWith("w ") || cmd.startsWith("send_raw")) {
        int sp = cmd.indexOf(' ');
        if (cat.sendRawHex(cmd.substring(sp + 1).c_str()))
            return rigctlOk();
        return rigctlErr(-1);
    }

    if (cmd == "\\get_info") {
        String info = "ESP32 CAT Remote Panel";
        const char* modelId = nullptr;
        if (channel && channel->radioModel[0])
            modelId = channel->radioModel;
        else if (cfg && cfg->radioModel[0])
            modelId = cfg->radioModel;
        if (modelId) {
            const RadioProfileInfo* p = radioProfileFind(modelId);
            info += " / ";
            info += p ? p->label : modelId;
            if (channel && channel->label[0]) {
                info += " [";
                info += channel->label;
                info += ']';
            }
        }
        return info + "\n" + rigctlOk();
    }

    if (cmd == "\\chk_vfo")
        return String("0\n") + rigctlOk();

    if (cmd == "\\dump_state" || cmd == "1" || cmd == "\\dump_caps")
        return String("0\n") + rigctlOk();

    Serial.printf("[rigctld] unknown cmd: %s\n", cmd.c_str());
    return rigctlErr(-4);
}
