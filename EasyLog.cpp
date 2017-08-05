#include "EasyLog.h"
#include "EasyLogUtils.cpp"

#include <assert.h>
#include <condition_variable>
#include <chrono>

#define MAX_LABEL_SIZE 256
#define MAX_FORMAT_ARGS_SIZE 4096

// Describes what messages are allowed based on visibility access level
// - This should be defined by your compiler settings, config file, etc..
#define EASY_LOG_VISIBILITY 2 // Highest form of access (Public, Protected, Private)

namespace EasyLog {

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
    /**
     *  Cleanup in our destructor
     */
    EasyLog::~EasyLog() {
        // Shut down the processing thread
        stopProcessing();
    }

    /**
    *   Start/Get singleton instance
    */
    EasyLog& EasyLog::getInstance() {
        static EasyLog s_easy_log;
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
            std::cout << "EasyLog::log supressed.  Message: " << msg << ". Visibility level: " << static_cast<int>(visibility) << ". EASY_LOG_VISIBILITY: " << EASY_LOG_VISIBILITY << std::endl;
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
    /**
    *  Are we currently processing any EasyLogItem(s)?
    */
    bool EasyLog::isRunning() {
        return getInstance()._running;
    }
    /** 
    *  Update max thread idle time in milliseconds
    */
    void EasyLog::setMaxThreadIdleTime(const int idle_time_ms) {
        if(idle_time_ms > 0) {
            getInstance()._max_thread_idle_time = idle_time_ms;
        }
    }
    /**
     *  Processes individual EasyLogItem(s)
     *  -  This method is responsible for registering the callback function used to send messages to user.
     *  -  See main.cpp for example case.
     */
    void EasyLog::registerProcessItemCallback(void (*callback)(EasyLogType type, EasyLogVisibility visibility, const char *msg, int64_t time_stamp), void* proc_item_data) {
        std::cerr << "EasyLog::registerProcessItemCallback" << std::endl;
        std::unique_lock<std::mutex> lock(getInstance()._log_queue_mutex);
        // Set callback
        getInstance()._on_proc_item_cb = callback;
    }
    /**
     *  Processes individual EasyLogItem(s)
     */
    void EasyLog::processItem(EasyLogType type, EasyLogVisibility visibility, const char *msg, int64_t time_stamp) {
        if(_on_proc_item_cb != nullptr) {
            _on_proc_item_cb(type, visibility, msg, time_stamp);
        } else {
            std::cerr << "Register a callback to get EasyLogItems!  You received a message." << std::endl;
        }
    }
    /** 
    *  Adds formatting and creates the EasyLogItem if direct is false
    */
    EasyLog::EasyLogItem EasyLog::formatLog(const char* tag, const char* fmt, EasyLogType type, EasyLogVisibility visibility, va_list arglist, bool add_timestamp) {

        std::string dateTime = EasyLogUtils::getTimeAsString(":", true);

        size_t fmt_len = strlen(fmt);
        unsigned long buffer_len = fmt_len + MAX_LABEL_SIZE;
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

    /**
    *  Enqueues an EasyLogItem
    */
    void EasyLog::addItem(EasyLogItem item) {
        std::cout << "EasyLog::addItem" << std::endl;
        std::unique_lock<std::mutex> lock(_log_queue_mutex);
        
        if (_shutting_down)
        {
            // EasyLog has shut down, abort
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
        std::unique_lock<std::mutex> lock(_log_queue_mutex);
        
        return _log_queue.size();
    }
    /**
    *   Stop all processing
    */
    void EasyLog::stopProcessing() {
        std::cout << "EasyLog::stopProcessing" << std::endl;
        std::unique_lock<std::mutex> lock(_log_queue_mutex);
        
        _running = false;
        _shutting_down = true;

        std::condition_variable cond;

        // Wait for full stop, if needed
        while (_log_queue_thread.is_active()) {
            cond.wait_for(lock,std::chrono::milliseconds(10));
        }
        
        std::cout << "EasyLog::stopProcessing" << std::endl;
    }
    /**
    *   Returns the next EasyLogItem on the queue
    */
    EasyLog::EasyLogItem EasyLog::getNextItem() {
        std::unique_lock<std::mutex> lock(_log_queue_mutex);

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
        this->time_stamp = EasyLogUtils::system_msec_time();
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
        std::cout << "EasyLogQueue Dispatch Start" << std::endl;

        _is_active = true;
        int64_t time_ms = EasyLogUtils::system_msec_time();

        while (_easy_log->_running) {
            if (_easy_log->processItems()) {
                time_ms = EasyLogUtils::system_msec_time();
            } else {
                int64_t curr_time_ms = EasyLogUtils::system_msec_time();
                int idle_time = int(curr_time_ms - time_ms);
                if (idle_time > _easy_log->_max_thread_idle_time) {
                    std::cout << "EasyLog timeout: " << double(idle_time)/1000.0f << " seconds" << std::endl;
                    _easy_log->_running = false;
                }
            }
        }

        std::cout << "EasyLogQueue Dispatch Stop" << std::endl;

        _is_active = false;
    }
    
    void EasyLog::EasyLogQueueThread::start() {
        _m_thread = std::thread(&EasyLogQueueThread::run, this);
        // We want our thread to process in the background
        _m_thread.detach();
    }

} // End EasyLog namespace
