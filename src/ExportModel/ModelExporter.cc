//------------------------------------------------------------------------------
//  ModelExporter.cc
//------------------------------------------------------------------------------
#include "ModelExporter.h"
#include "ExportUtil/Log.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <assert.h>


namespace OryolTools {

//------------------------------------------------------------------------------
bool
ModelExporter::Import(const std::string& path) {
    assert(!path.empty());

    this->importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
    this->scene = importer.ReadFile(path,
        // FIXME: make processing flags configurable via TOML
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_PreTransformVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_SortByPType |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData |
        aiProcess_GenUVCoords);
    if (!this->scene) {
        Log::Warn("Failed to import file '%s': %s\n", path.c_str(), importer.GetErrorString());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
const aiScene*
ModelExporter::GetScene() const {
    assert(this->scene);
    return this->scene;
}

} // namespace OryolTools