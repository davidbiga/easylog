# easylog
EasyLog is a simple C++ logging mechanism that can be used for all debugging and production logging related purposes.
# Set Up
1. `git clone https://github.com/davidbiga/easylog.git`
2. `#include "EasyLog.h" where you'd like to register the callback.`
3. Register the callback to EasyLog - `EasyLog::EasyLog::getInstance().registerProcessItemCallback(cb, cb_instance)`
4. EasyLog!

# How-to-use
1. `#include "EasyLog.h"` wherever you'd like to EasyLog.
2. call `EasyLog::EasyLog::getInstance().log(EasyLog::EasyLogType, EasyLog::EasyLogVisibility, "This is EasyLog!");`

For exact implementation and usage, see `main.cpp`.

# Important EasyLog Details
#### `EASY_LOG_VISIBILITY` - Tells access levels for EasyLogItem(s).  This is useful for internal systems that want to limit certain messages from being sent to a client. `0 = Public` - `1 = Protected` - `2 = Private`.  If the system logs as `EasyLogVisibility::Private` and `EASY_LOG_VISIBILITY` is set to `1`, this message will be supressed.
