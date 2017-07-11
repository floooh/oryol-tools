#pragma once
//------------------------------------------------------------------------------
/**
    @class AssimpLoader
    @brief load 3D data into IRep via assimp
*/
#include "assimp/Importer.hpp"
#include "IRep.h"

struct AssimpLoader {
    /// load file into intermediate representation
    void Load(const std::string& path, IRep& irep);

    /// write loading result into intermediate representation
    void toIRep(IRep& irep);
    /// write vertex components to irep
    void writeVertexComponents(IRep& irep);
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
};