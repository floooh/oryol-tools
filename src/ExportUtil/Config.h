#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::Config
    @brief config file wrapper
*/
#include "ExportUtil/Vertex.h"
#include "cpptoml.h"
#include <string>

namespace OryolTools {

class Config {
public:
    /// load config file
    bool Load(const std::string& path);
    /// get assimp scene processing flags
    unsigned int GetAiProcessFlags() const;
    /// get the remove component flags (PP_RVC_FLAGS)
    unsigned int GetAiProcessRemoveComponentsFlags() const;
    /// get the remove primitive type flag mast (PP_SBP_REMOVE)
    unsigned int GetAiProcessSortByPTypeRemoveFlags() const;
    /// get the vertex layout
    VertexLayout GetLayout() const;

private:
    cpptoml::table config;
};

} // namespace OryolTools