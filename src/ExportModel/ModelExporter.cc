//------------------------------------------------------------------------------
//  ModelExporter.cc
//------------------------------------------------------------------------------
#include "ModelExporter.h"
#include "ExportUtil/Log.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <cassert>

namespace OryolTools {

//------------------------------------------------------------------------------
void
ModelExporter::SetAiProcessFlags(unsigned int flags) {
    this->aiProcessFlags = flags;
}

//------------------------------------------------------------------------------
void
ModelExporter::SetAiProcessRemoveComponentsFlags(unsigned int flags) {
    this->aiProcessRemoveComponentsFlags = flags;
}

//------------------------------------------------------------------------------
void
ModelExporter::SetAiProcessSortByPTypeRemoveFlags(unsigned int flags) {
    this->aiProcessSortByPTypeRemoveFlags = flags;
}

//------------------------------------------------------------------------------
bool
ModelExporter::Import(const std::string& path) {
    assert(!path.empty());

    this->importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, this->aiProcessRemoveComponentsFlags);
    this->importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, this->aiProcessSortByPTypeRemoveFlags);

    this->scene = importer.ReadFile(path, aiProcessFlags);
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