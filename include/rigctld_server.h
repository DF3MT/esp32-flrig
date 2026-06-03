#pragma once

#include "cat_controller.h"
#include "config.h"
#include "hamlib_tcp_server.h"

using RigctldServer = HamlibTcpServer;

#define rigctlOk  hamlibRprtOk
#define rigctlErr hamlibRprtErr

String dispatchRigctl(const String& line, CatController& cat, RadioState& state,
                      const AppConfig* cfg = nullptr);
