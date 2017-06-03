#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::Log
    @brief static logging functions
*/
#include <cstdio>
#include <cassert>

namespace OryolTools {

class Log {
public:
    /// print normal info
    static void Info(const char* str, ...);
    /// print warning (to stderr)
    static void Warn(const char* str, ...);
    /// display an error message and terminate the program
    static void Fatal(const char* str, ...);
    /// if condition is true, print error message and fail
    static void FailIf(bool cond, const char* str, ...);
};

} // namespace OryolTools
