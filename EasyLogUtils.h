#include <iostream>
#include <sys/time.h>

namespace EasyLogUtils {
    std::string getTimeAsString(const char *separator, bool includeMilliseconds);
    int64_t system_usec_time();
    int64_t system_msec_time();
} // End EasyLogUtils namespace