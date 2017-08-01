//===-- EasyLog.h - EasyLog class definition -------*- C++ -*-===//
///
/// @file
/// @brief This file contains the definitions for all use cases of EasyLog.
///     EasyLog is meant to be a utility that allows you to collect any form of data you might need.
///
//===----------------------------------------------------------------------===//

#ifndef __EasyLog__
#define __EasyLog__

#include <queue>
#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <sys/time.h>

/**
*   EasyLogType
*       Defined the type of EasyLog message that is being sent.
**/
enum class EasyLogType {
    Error,
    Info,
    General,
    Events,
    Stats,
    Timing,
    Num_Types
};

/**
*   EasyLogVisibility
*       Defines the type of EasyLogVisibility.  One might use this to determining who can see 
*       what types of messages.
**/
enum class EasyLogVisibility {
    Public,
    Protected,
    Private
};

class EasyLog {
public:
    virtual ~EasyLog();
    // Get or start singleton instance
    static EasyLog& getInstance();
    // Strings with args
    static void log(EasyLogType type, EasyLogVisibility visibility, const char* msg, ...);
    // Pre-formatted strings
    static void log(EasyLogType type, EasyLogVisibility visibility, const std::string &msg);
    // Formats message with callstack and forwards synchronously, then throws an exception - uses the error channel
    static void logFatal(EasyLogVisibility visibility, const char* msg, ...);
    // Return state of _running - This determines whether our EasyLogQueueThread is processing
    bool isRunning();
    // For testing purposes, wait for timeout on QueueThread
    void waitForTimeout();
    // Allow adjustment of thread idle time
    void setMaxThreadIdleTime(const int idle_time_ms);
private:
    EasyLog() = default;
    // Stop the processing of log items safely
    void stopProcessing();
    // Applications using EasyLog need to implement the processItem function
    virtual void processItem(EasyLogType type, EasyLogVisibility visibility, const char *msg, int64_t time_stamp) = 0;
    // Message item that contains data associated with specific log
    struct EasyLogItem
    {
        EasyLogItem(EasyLogType type, EasyLogVisibility visibility, const char *msg, size_t msg_size);
        std::string msg;
        EasyLogType type;
        EasyLogVisibility visibility;
        int64_t time_stamp;
    };
    // Manage the EasyLog dispatch
    class EasyLogQueueThread {
        public:
            EasyLogQueueThread(EasyLog *easy_log);
            virtual void run();
            bool is_active() { return _is_active; };
        protected:
            virtual const char *getThreadName() { return "EasyLogQueueThread"; }
        private:
            bool _is_active;
            EasyLog *_easy_log;
    };
    
    // utility to format the log string and enqueue it
    EasyLog::EasyLogItem formatLog(const char *tag, const char *fmt, EasyLogType type, EasyLogVisibility visibility, va_list arglist, bool add_timestamp);
    // Process items and return the number processed
    int processItems();
    // Enqueue a EasyLogItem safely
    void addItem(EasyLogItem item);
    // Check the queue size safely
    size_t queueSize() const;
    // Get the next item to process and remove it from the queue safely
    EasyLog::EasyLogItem getNextItem();
    // Is EasyLogQueueThread running?
    bool _running;
    // Is EasyLogQueueThread shutting down?
    bool _shutting_down;
    // Max idle time in ms
    int _max_thread_idle_time;
    // Queued EasyLogItem
    std::queue<EasyLogItem> _log_queue;
    mutable std::mutex _log_queue_mutex;
    // Queue thread instance
    EasyLogQueueThread _log_queue_thread;
    // Current active EasyLog instance - can only be one
    static EasyLog* s_easy_log;
};

#endif