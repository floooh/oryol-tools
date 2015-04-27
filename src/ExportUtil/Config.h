#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::Config
    @brief config file wrapper
*/
#include "cpptoml.h"
#include <string>

namespace OryolTools {

class Config {
public:
    /// load config file
    bool Load(const std::string& path);
    /// get assimp scene processing flags
    unsigned int GetAiProcessFlags() const;

private:
    cpptoml::table config;
};

} // namespace OryolTools