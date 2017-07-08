#pragma once
//------------------------------------------------------------------------------
/**
    @class IRepJsonDumper
    @brief dump the intermediate representation to JSON
*/
#include "IRep.h"
#include <string>

struct IRepJsonDumper {
    /// dump IRep object to JSON
    static std::string DumpIRep(const IRep& irep);
    /// dump an IRepProcessor template
    static std::string DumpIRepProcessor(const IRep& irep);
};