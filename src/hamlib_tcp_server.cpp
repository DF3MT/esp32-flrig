#include "hamlib_tcp_server.h"

String hamlibRprtOk() { return "RPRT 0\n"; }
String hamlibRprtErr(int code) { return "RPRT " + String(code) + "\n"; }

bool HamlibTcpServer::begin(uint16_t port, const char* logTag) {
    _port = port;
    _tag = logTag ? logTag : "hamlib";
    _server = new WiFiServer(port);
    _server->begin();
    _server->setNoDelay(true);
    Serial.printf("[%s] listening on port %u\n", _tag, port);
    return true;
}

void HamlibTcpServer::loop() {
    if (!_server) return;

    if (!_client || !_client.connected()) {
        _client = _server->available();
        if (_client) {
            _client.setNoDelay(true);
            _buffer = "";
            Serial.printf("[%s] client connected\n", _tag);
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

String HamlibTcpServer::processLine(const String& line) {
    if (_handler) return _handler(line);

    String cmd = line;
    cmd.trim();
    if (cmd == "q" || cmd == "quit") {
        _client.stop();
        return hamlibRprtOk();
    }
    return hamlibRprtErr(-1);
}
