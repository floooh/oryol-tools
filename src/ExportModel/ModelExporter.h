#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::ModelExporter
    @brief export 3D models to Oryol file format using assimp
*/
#include <string>
#include "assimp/Importer.hpp"

namespace OryolTools {

class ModelExporter {
public:
    /// perform export
    bool Import(const std::string& path);
    /// get pointer to imported scene
    const aiScene* GetScene() const;

private:
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
};

} // namespace OryolTools

