#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

/** Hamlib-style TCP daemon (rigctld / rotctld text protocol). */
class HamlibTcpServer {
public:
    using CommandHandler = std::function<String(const String& cmd)>;

    bool begin(uint16_t port, const char* logTag = "hamlib");
    void loop();
    void setHandler(CommandHandler h) { _handler = h; }

private:
    WiFiServer* _server = nullptr;
    WiFiClient  _client;
    String      _buffer;
    CommandHandler _handler;
    uint16_t    _port = 0;
    const char* _tag = "hamlib";

    String processLine(const String& line);
};

String hamlibRprtOk();
String hamlibRprtErr(int code);
