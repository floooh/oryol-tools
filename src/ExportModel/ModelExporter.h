#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::ModelExporter
    @brief export 3D models to Oryol file format using assimp
*/
#include <string>
#include "assimp/Importer.hpp"
#include "ExportUtil/Mesh.h"
#include "glm/vec3.hpp"

namespace OryolTools {

class ModelExporter {
public:
    /// destructor
    ~ModelExporter();
    /// set aiProcess scene processing flags
    void SetAiProcessFlags(unsigned int flags);
    /// set component removal flags (PP_RVC_FLAGS)
    void SetAiProcessRemoveComponentsFlags(unsigned int flags);
    /// set primitive type removal flags (PP_SBP_REMOVE)
    void SetAiProcessSortByPTypeRemoveFlags(unsigned int flags);
    /// set requested index size (must be 2 or 4 for 16-bit or 32-bit indices, default: 2)
    void SetIndexSize(int size);
    /// set vertex layout description
    void SetVertexLayout(const VertexLayout& layout);
    /// import data
    bool ImportScene(const std::string& path);
    /// export converted import data as complete model
    bool ExportModel(const std::string& path);
    /// export converted import data as mesh-only
    bool ExportMesh(const std::string& path);
    /// discard content
    void DiscardContent();
    /// get pointer to imported scene
    const aiScene* GetScene() const;
    /// get exported mesh
    const Mesh& GetMesh() const;
    /// compute bounding box of loaded geometry
    void ComputeBoundingBox();
    /// get bounding box min
    const glm::vec3& GetBoundingBoxMin() const;
    /// get bounding box max
    const glm::vec3& GetBoundingBoxMax() const;

private:
    /// convert vertex data into embedded VertexBuffer object
    bool exportVertices();
    /// convert index data into embedded IndexBuffer object
    bool exportIndices();
    /// export primitive groups into mesh object
    bool exportPrimGroups();

    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    unsigned int aiProcessFlags = 0;
    unsigned int aiProcessRemoveComponentsFlags = 0;
    unsigned int aiProcessSortByPTypeRemoveFlags = 0;
    int indexSize = 2;
    VertexLayout requestedVertexLayout;
    Mesh mesh;
    glm::vec3 boxMin;
    glm::vec3 boxMax;
};

} // namespace OryolTools

