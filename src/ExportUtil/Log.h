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
};

} // namespace OryolTools
