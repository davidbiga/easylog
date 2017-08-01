#include "EasyLog.h"

#define MAX_THREAD_IDLE_TIME 3000
#define MAX_LABEL_SIZE 256
#define MAX_FORMAT_ARGS_SIZE 4096

// Describes what messages are allowed based on visibility access level
#define EASY_LOG_VISIBILITY 2 // Highest form of access (Public, Protected, Private)

// TODO add labels to EasyLogItem.
std::vector<const char*> easy_log_type_tags =
{
    "ERROR",
    "INFO",
    "GENERAL",
    "EVENTS",
    "STATS",
    "TIMING"
};

// Cleanup in our destructor
EasyLog::~EasyLog() {
    // Shut down the processing thread
    stopProcessing();
}

/**
*   Start/Get singleton instance
*/
EasyLog& EasyLog::getInstance() {
    // Lets see if EasyLog was initialized
    if(s_easy_log == nullptr) {
        static EasyLog s_easy_log;
        _running = false;
        _shutting_down = false;
        _max_thread_idle_time = MAX_THREAD_IDLE_TIME;
        _log_queue_thread(s_easy_log);
    }
    return s_easy_log;
}

/**
 *  log(type, visibility, message, va_args)
 */
void EasyLog::log(EasyLogType log_type, EasyLogVisibility visibility, const char* msg, ...) {
    // Missing tag for EasyLogType enum above
    assert(easy_log_type_tags.size() == static_cast<size_t>(EasyLogType::Num_Types));

    // Validate visibility level based on compiler setting
    // We want to check for greater-than because if EASY_LOG_VISIBILITY is set to 2 (private)
    // it should also receive EasyLog's that have visbility of public and protected because of its clearance
    if (static_cast<int>(visibility) > EASY_LOG_VISIBILITY) {
        std::cout << "MonitorLog::log supressed.  Message: " << msg << ". Visibility level: " << static_cast<int>(visibility) << ". EASY_LOG_VISIBILITY: " << EASY_LOG_VISIBILITY << std::endl;
        return;
    }

    va_list arglist;
    va_start(arglist, msg);
    // Format log and return an EasyLogItem to add to queue
    EasyLogItem item = getInstance().formatLog(easy_log_type_tags[static_cast<int>(log_type)], msg, log_type, visibility, arglist, log_type != EasyLogType::Timing);
    getInstance().addItem(item);

    va_end(arglist);
}
/**
 *  log(type, visibility, message)
 */
void EasyLog::log(EasyLogType log_type, EasyLogVisibility visibility, const std::string& msg) {
    log(log_type, visibility, msg.c_str());
}
// Are we currently processing any EasyLogItem(s)?
bool EasyLog::isRunning() {
    return getInstance()._running;
}
// Update max thread idle time in milliseconds
void setMaxThreadIdleTime(const int idle_time_ms) {
    if(idle_time_ms > 0) {
        getInstance()._max_thread_idle_time = idle_time_ms;
    }
}

// Adds formatting and creates the EasyLogItem if direct is false
EasyLog::EasyLogItem EasyLogLog::formatLog(const char* tag, const char* fmt, EasyLogType type, EasyLogVisibility visibility, va_list arglist, bool add_timestamp) {

    std::string dateTime = getTimeAsString(":", true);

    size_t fmt_len = strlen(fmt);
    int buffer_len = fmt_len + MAX_LABEL_SIZE;
    char label [buffer_len];
    int log_len = 0;

    char log [buffer_len + MAX_FORMAT_ARGS_SIZE];
    if (add_timestamp) {
        snprintf(label, buffer_len, "[%s %s] %s\n", tag, dateTime.c_str(), fmt);
        log_len = vsnprintf(log, sizeof(log), label, arglist);
    } else {
        log_len = vsnprintf(log, sizeof(log), fmt, arglist);
    }

    return EasyLog::EasyLogItem(type, visibility, log, log_len);
}

// Enqueues a EasyLogItem
void EasyLog::addItem(EasyLogItem item) {
    std::cout << "EasyLog::addItem" << std::endl;
    std::unique_lock<utility::Mutex> lock(_log_queue_mutex);

    if (_shutting_down) {
        //monitor logging has shut down, abort
        return;
    }

    _log_queue.push(item);

    std::cout << "EasyLog::addItem size: " <<  _log_queue.size() << std::endl;

    if (!_running) {
        _running = true;
        _log_queue_thread.start();
    }
}
/**
*   Return queue size
*/
size_t EasyLog::queueSize() const {
    std::unique_lock<utility::Mutex> lock(_log_queue_mutex);
    return _log_queue.size();
}
/**
*   Stop all processing
*/
void EasyLog::stopProcessing() {

    std::cout << "EasyLog::stopProcessing" << std::endl;

    std::unique_lock<utility::Mutex> lock(_log_queue_mutex);
    _running = false;
    _shutting_down = true;

    utility::Condition cond;

    // Wait for full stop, if needed
    while (_log_queue_thread.is_active()) {
        cond.timedWait(_log_queue_mutex, 10);
    }
    
    std::cout << "EasyLog::stopProcessing" << std::endl;
}
/**
*   Returns the next EasyLogItem on the queue
*/
EasyLog::EasyLogItem EasyLog::getNextItem() {

    std::unique_lock<utility::Mutex> lock(_log_queue_mutex);

    assert(_log_queue.size() > 0); //do not call on an empty queue
    EasyLog::EasyLogItem item = _log_queue.front();
    _log_queue.pop();
    return item;
}
/**
*   Process all EasyLogItem(s) on the queue
*/
int EasyLog::processItems() {

    int count = 0;
    while (queueSize() > 0) {
        EasyLogItem item = getNextItem();

        if (_shutting_down)  {
            break;
        }

        processItem(item.type, item.visibility, item.msg.c_str(), item.time_stamp);

        ++count;
        _log_queue_thread.yield();
    }
    return count;
}

/**
*
*   ================================================= EasyLogItem ========================================
*
*/

EasyLog::EasyLogItem::EasyLogItem(EasyLogType type, EasyLogVisibility visibility, const char* msg, size_t msg_len) {
    this->msg.assign(msg, msg_len);
    this->type = type;
    this->visibility = visibility;
    this->time_stamp = system_msec_time();
}

/**
*
*   ================================================= EasyLogQueueThread ========================================
*
*/

EasyLog::EasyLogQueueThread::EasyLogQueueThread(EasyLog* easy_log)
{
    _easy_log = easy_log;
    _is_active = false;
}

void EasyLog::EasyLogQueueThread::run() {
    std::cout << "EasyLogQueue Dispatch Start");

    _is_active = true;
    int64_t time_ms = system_msec_time();

    while (_easy_log->_running) {
        if (_easy_log->processItems()) {
            time_ms = system_msec_time();
        } else {
            int64_t curr_time_ms = system_msec_time();
            int idle_time = int(curr_time_ms - time_ms);
            if (idle_time > _monitor_log->_max_thread_idle_time) {
                std::cout << "MonitorLog timeout: " << double(idle_time)/1000.0f << " seconds" << std::endl;
                _easy_log->_running = false;
            }
        }
        yield(); // Give up the processor
    }

    std::cout << "EasyLogQueue Dispatch Stop";

    _is_active = false;
}


/**
*
*   ================================================= HELPER(s) ========================================
*
*/

// Returns time as string
std::string getTimeAsString(const char *separator, bool includeMilliseconds) const
{
    time_t now = _tval.tv_sec;

    tm *ltm = localtime(&now);
    char dateTime[256];
    snprintf(dateTime, sizeof(dateTime)-1, "%.02i%s%.02i%s%.02i", ltm->tm_hour, separator,  ltm->tm_min, separator, ltm->tm_sec);

    if (!includeMilliseconds)
        return std::string(dateTime);

    int milliseconds = _tval.tv_usec / 1000;
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