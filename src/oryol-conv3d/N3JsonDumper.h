#pragma once
//------------------------------------------------------------------------------
/**
    @class N3JsonDumper
    @brief dump N3Loader to JSON
*/
#include "N3Loader.h"
#include <string>

struct N3JsonDumper {
    /// dump N3Loader to JSON string
    static std::string Dump(const N3Loader& n3);
};
