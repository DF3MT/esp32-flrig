#pragma once

#include "cat_controller.h"
#include <WiFi.h>
#include <functional>

class RigctldServer {
public:
    using CommandHandler = std::function<String(const String& cmd)>;

    bool begin(uint16_t port = RIGCTLD_PORT);
    void loop();
    void setHandler(CommandHandler h) { _handler = h; }

private:
    WiFiServer* _server = nullptr;
    WiFiClient  _client;
    String      _buffer;
    CommandHandler _handler;
    uint16_t    _port;

    void handleClient();
    String processLine(const String& line);
};

String rigctlOk();
String rigctlErr(int code);

// Built-in rigctl command dispatcher (works with CatController)
String dispatchRigctl(const String& line, CatController& cat, RadioState& state);
