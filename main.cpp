#include "EasyLog.h"

#include <iostream>

class EasyLogSDK : public EasyLog::EasyLog {
public:
    virtual void processItem(::EasyLog::EasyLogType type, ::EasyLog::EasyLogVisibility visibility, const char *msg, int64_t time_stamp);
};

/**
 * EasyLogSDK Implementation
 */
void EasyLogSDK::processItem(::EasyLog::EasyLogType type, ::EasyLog::EasyLogVisibility visibility, const char *msg, int64_t time_stamp){
    // ############
    //  This is critical for the user to override processItem in their subclass of EasyLog in order to get log messages
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

int
main(int argc, const char* argv[]) {
    EasyLogSDK::EasyLog &easy_log_sdk = EasyLogSDK::EasyLog::getInstance();
    //EasyLog::EasyLog &easy_log = EasyLog::EasyLog::getInstance();
    easy_log_sdk.log(EasyLog::EasyLogType::General, EasyLog::EasyLogVisibility::Public, "Testing public general EasyLog message");
    easy_log_sdk.log(EasyLog::EasyLogType::Stats, EasyLog::EasyLogVisibility::Protected, "Testing protected stats EasyLog message");
    easy_log_sdk.log(EasyLog::EasyLogType::Error, EasyLog::EasyLogVisibility::Private, "Testing private error EasyLog message");
    // Wait for EasyLogQueue Dispatch to finish
    while(easy_log_sdk.isRunning()){}
    // End program
    return 0;
}
