# easylog
EasyLog is a simple C++ logging mechanism that can be used for all debugging and production logging related purposes.
# Set Up
1. `git clone https://github.com/davidbiga/easylog.git`
2. `#include "EasyLog.h" where you'd like to register the callback.
3. Register the callback to EasyLog - `EasyLog::EasyLog::getInstance().registerProcessItemCallback(cb, cb_instance)`
4. EasyLog!

# How-to-use
1. `#include "EasyLog.h"` wherever you'd like to log.
2. call `EasyLog::EasyLog::getInstance().log(EasyLog::EasyLogType, EasyLog::EasyLogVisibility, "This is EasyLog!");`

For exact implementation and usage, see `main.cpp`.