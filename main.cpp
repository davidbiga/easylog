#include "EasyLog.h"

#include <iostream>

/**
 * EasyLogSDK Implementation
 */

class EasyLogSDK {
public:
    static void processItemCallback(::EasyLog::EasyLogType type, ::EasyLog::EasyLogVisibility visibility, const char *msg, int64_t time_stamp);
};

void EasyLogSDK::processItemCallback(::EasyLog::EasyLogType type, ::EasyLog::EasyLogVisibility visibility, const char *msg, int64_t time_stamp){
    // ############
    //  This is critical for the user to implement a callback for EasyLogItem(s) to be received
    // ############
    std::string n_type, n_visibility;
    
    switch (type) {
        case ::EasyLog::EasyLogType::Error: {
            n_type = "Error";
            break;
        }
        case ::EasyLog::EasyLogType::Info: {
            n_type = "Info";
            break;
        }
        case ::EasyLog::EasyLogType::General: {
            n_type = "General";
            break;
        }
        case ::EasyLog::EasyLogType::Events: {
            n_type = "Events";
            break;
        }
        case ::EasyLog::EasyLogType::Stats: {
            n_type = "Stats";
            break;
        }
        case ::EasyLog::EasyLogType::Timing: {
            n_type = "Timing";
            break;
        }
        default: {
            n_type = "Unkown EasyLogType";
            break;
        }
    }
    
    switch (visibility) {
        case ::EasyLog::EasyLogVisibility::Public: {
            n_visibility = "Public";
            break;
        }
        case ::EasyLog::EasyLogVisibility::Protected: {
            n_visibility = "Protected";
            break;
        }
        case ::EasyLog::EasyLogVisibility::Private: {
            n_visibility = "Private";
            break;
        }
        default: {
            n_type = "Unkown EasyLogVisibility";
            break;
        }
    }
    
    std::cerr << msg << " [EasyLogType:] " << n_type << " [EasyLogVisibility:] " << n_visibility << " [TimeStamp:] " << time_stamp << std::endl;
}
/**
*   End EasyLogSDK Implementation
*/

static EasyLogSDK easy_log_sdk;

/**
 *  Entry point
 */
int
main(int argc, const char* argv[]) {
    EasyLog::EasyLog &easy_log = EasyLog::EasyLog::getInstance();
    easy_log.registerProcessItemCallback(EasyLogSDK::processItemCallback, &easy_log_sdk);
    //EasyLog::EasyLog &easy_log = EasyLog::EasyLog::getInstance();
    easy_log.log(EasyLog::EasyLogType::General, EasyLog::EasyLogVisibility::Public, "Testing public general EasyLog message");
    easy_log.log(EasyLog::EasyLogType::Stats, EasyLog::EasyLogVisibility::Protected, "Testing protected stats EasyLog message");
    easy_log.log(EasyLog::EasyLogType::Error, EasyLog::EasyLogVisibility::Private, "Testing private error EasyLog message");
    // Wait for EasyLogQueue Dispatch to finish
    while(easy_log.isRunning()){}
    // End program
    return 0;
}
