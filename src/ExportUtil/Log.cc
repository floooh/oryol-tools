//------------------------------------------------------------------------------
//  Log.cc
//------------------------------------------------------------------------------
#include "Log.h"
#include <stdarg.h>

namespace OryolTools {

//------------------------------------------------------------------------------
void 
Log::Info(const char* str, ...) {
    va_list args;
    va_start(args, str);
    std::vprintf(str, args);
    va_end(args);
}

//------------------------------------------------------------------------------
void 
Log::Warn(const char* str, ...) {
    va_list args;
    va_start(args, str);
    std::fprintf(stderr, "[warn] ");
    std::vfprintf(stderr, str, args);
    va_end(args);
}

//------------------------------------------------------------------------------
void 
Log::Fatal(const char* str, ...) {
    va_list args;
    va_start(args, str);
    std::fprintf(stderr, "[error] ");
    std::vfprintf(stderr, str, args);
    va_end(args);
    std::fflush(stdout);
    std::fflush(stderr);
    exit(10);
}

} // namespace OryolTools

