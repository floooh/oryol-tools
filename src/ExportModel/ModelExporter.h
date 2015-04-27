#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::ModelExporter
    @brief export 3D models to Oryol file format using assimp
*/
#include <string>
#include "assimp/Importer.hpp"
#include "ExportUtil/Vertex.h"

namespace OryolTools {

class ModelExporter {
public:
    /// set aiProcess scene processing flags
    void SetAiProcessFlags(unsigned int flags);
    /// set component removal flags (PP_RVC_FLAGS)
    void SetAiProcessRemoveComponentsFlags(unsigned int flags);
    /// set primitive type removal flags (PP_SBP_REMOVE)
    void SetAiProcessSortByPTypeRemoveFlags(unsigned int flags);
    /// set vertex layout description
    void SetVertexLayout(const VertexLayout& layout);
    /// perform export
    bool Import(const std::string& path);
    /// get pointer to imported scene
    const aiScene* GetScene() const;

private:
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    unsigned int aiProcessFlags = 0;
    unsigned int aiProcessRemoveComponentsFlags = 0;
    unsigned int aiProcessSortByPTypeRemoveFlags = 0;
    VertexLayout vertexLayout;
};

} // namespace OryolTools

