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
    static std::string Dump(const IRep& irep);
};