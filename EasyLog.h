//===-- EasyLog.h - EasyLog class definition -------*- C++ -*-===//
///
/// @file
/// @brief This file contains the definitions for all use cases of EasyLog.
///     EasyLog is meant to be a utility that allows you to collect any form of data you might need.
///
//===----------------------------------------------------------------------===//

#ifndef __EasyLog__
#define __EasyLog__

#include <string>

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
    Num_types
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
    protected:
    private:
}

#endif