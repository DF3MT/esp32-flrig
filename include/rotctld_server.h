#pragma once

#include "rotor_controller.h"
#include "config.h"

String dispatchRotctl(const String& line, RotorController& rotor, const AppConfig* cfg = nullptr);
