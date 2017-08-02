#include "EasyLogUtils.h"

#define MILLION 1000000

namespace EasyLogUtils {
    /**
    *  Returns time as string
    */
    std::string getTimeAsString(const char *separator, bool includeMilliseconds)
    {
        time_t now;
        time(&now);

        tm *ltm = localtime(&now);
        char dateTime[256];
        snprintf(dateTime, sizeof(dateTime)-1, "%.02i%s%.02i%s%.02i", ltm->tm_hour, separator,  ltm->tm_min, separator, ltm->tm_sec);

        if (!includeMilliseconds)
            return std::string(dateTime);

        int milliseconds = now / 1000;
        char dateMilliTime[256];
        snprintf(dateMilliTime, sizeof(dateMilliTime)-1, "%s%s%.03i", dateTime, separator, milliseconds);

        return std::string(dateMilliTime);
    }

    /**
    *   Time Utilities
    */
    int64_t
    system_usec_time()
    {
        timeval st_t;
        gettimeofday(&st_t, 0);
        return int64_t(st_t.tv_sec)*MILLION + st_t.tv_usec;
    }

    int64_t
    system_msec_time()
    {
        return system_usec_time() / 1000;
    }
    /**
    *   End Time Utilities
    */
} // End EasyLogUtils namespace