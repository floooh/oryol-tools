//------------------------------------------------------------------------------
//  ModelExporter.cc
//------------------------------------------------------------------------------
#include "ModelExporter.h"
#include "ExportUtil/Log.h"
#include "ExportUtil/MeshSaver.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <cassert>
#include <cfloat>

using namespace std;

namespace OryolTools {

//------------------------------------------------------------------------------
ModelExporter::~ModelExporter() {
    this->scene = 0;
    if (this->mesh.VertexBuffer.IsValid()) {
        this->mesh.VertexBuffer.Discard();
    }
    if (this->mesh.IndexBuffer.IsValid()) {
        this->mesh.IndexBuffer.Discard();
    }
    this->mesh.PrimGroups.clear();
}

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
void
ModelExporter::SetIndexSize(int size) {
    if ((size != 2) && (size != 4)) {
        Log::Fatal("Invalid index size, must be 2 or 4\n");
    }
    this->indexSize = size;
}

//------------------------------------------------------------------------------
void
ModelExporter::SetVertexLayout(const VertexLayout& layout) {
    this->requestedVertexLayout = layout;
}

//------------------------------------------------------------------------------
bool
ModelExporter::ImportScene(const std::string& path) {
    assert(!path.empty());

    this->importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, this->aiProcessRemoveComponentsFlags);
    this->importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, this->aiProcessSortByPTypeRemoveFlags);

    this->scene = importer.ReadFile(path, aiProcessFlags);
    if (!this->scene) {
        Log::Warn("Failed to import file '%s': %s\n", path.c_str(), importer.GetErrorString());
        return false;
    }
    Log::Info("Imported file '%s'\n", path.c_str());
    return true;
}

//------------------------------------------------------------------------------
bool
ModelExporter::ExportMesh(const std::string& path) {
    assert(!path.empty());
    assert(!this->mesh.VertexBuffer.IsValid());
    assert(!this->mesh.IndexBuffer.IsValid());
    assert(this->mesh.PrimGroups.empty());

    this->exportVertices();
    this->exportIndices();
    this->exportPrimGroups();

    FILE* fp = fopen(path.c_str(), "wb");
    if (fp) {
        size_t size = MeshSaver::Save(this->mesh, fp);
        fclose(fp);
        Log::Info("Exported mesh to '%s' (%d bytes)\n", path.c_str(), size);
        return true;
    }
    else {
        Log::Fatal("Failed to open file '%s'\n", path.c_str());
        return false;
    }
}

//------------------------------------------------------------------------------
bool
ModelExporter::ExportModel(const std::string& path) {
    assert(!path.empty());
    assert(!this->mesh.VertexBuffer.IsValid());
    assert(!this->mesh.IndexBuffer.IsValid());
    assert(this->mesh.PrimGroups.empty());

    // FIXME!
    return false;
}

//------------------------------------------------------------------------------
void
ModelExporter::ComputeBoundingBox() {
    this->boxMin = glm::vec3(FLT_MAX);
    this->boxMax = glm::vec3(-FLT_MAX);
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        for (unsigned int vertIndex = 0; vertIndex < curMesh->mNumVertices; vertIndex++) {
            const aiVector3D& pos = curMesh->mVertices[vertIndex];
            if (pos.x < this->boxMin.x) this->boxMin.x = pos.x;
            if (pos.y < this->boxMin.y) this->boxMin.y = pos.y;
            if (pos.z < this->boxMin.z) this->boxMin.z = pos.z;
            if (pos.x > this->boxMax.x) this->boxMax.x = pos.x;
            if (pos.y > this->boxMax.y) this->boxMax.y = pos.y;
            if (pos.z > this->boxMax.z) this->boxMax.z = pos.z;
        }
    }
}

//------------------------------------------------------------------------------
const glm::vec3&
ModelExporter::GetBoundingBoxMin() const {
    return this->boxMin;
}

//------------------------------------------------------------------------------
const glm::vec3&
ModelExporter::GetBoundingBoxMax() const {
    return this->boxMax;
}

//------------------------------------------------------------------------------
bool
ModelExporter::exportVertices() {
    assert(nullptr != this->scene);
    assert(nullptr != this->scene->mMeshes);
    assert(!this->mesh.VertexBuffer.IsValid());

    // compute overall number of vertices
    int allNumVertices = 0;
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        allNumVertices += curMesh->mNumVertices;
    }

    // get the actual vertex layout (requested vertex components AND provided vertex components)
    VertexLayout layout;
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        if (this->requestedVertexLayout.HasAttr(VertexAttr::Position)) {
            layout.Components[VertexAttr::Position] = this->requestedVertexLayout.Components[VertexAttr::Position];
        }
        if (curMesh->HasNormals() && this->requestedVertexLayout.HasAttr(VertexAttr::Normal)) {
            layout.Components[VertexAttr::Normal] = this->requestedVertexLayout.Components[VertexAttr::Normal];
        }
        if (curMesh->HasTangentsAndBitangents() && this->requestedVertexLayout.HasAttr(VertexAttr::Tangent)) {
            layout.Components[VertexAttr::Tangent] = this->requestedVertexLayout.Components[VertexAttr::Tangent];
        }
        if (curMesh->HasTangentsAndBitangents() && this->requestedVertexLayout.HasAttr(VertexAttr::Binormal)) {
            layout.Components[VertexAttr::Binormal] = this->requestedVertexLayout.Components[VertexAttr::Binormal];
        }
        for (int i = 0; i < 4; i++) {
            const VertexAttr::Code attr = (VertexAttr::Code)(VertexAttr::TexCoord0 + i);
            if (curMesh->HasTextureCoords(i) && this->requestedVertexLayout.HasAttr(attr)) {
                layout.Components[attr] = this->requestedVertexLayout.Components[attr];
            }
        }
        for (int i = 0; i < 2; i++) {
            const VertexAttr::Code attr = (VertexAttr::Code)(VertexAttr::Color0 + i);
            if (curMesh->HasVertexColors(i) && this->requestedVertexLayout.HasAttr(attr)) {
                layout.Components[attr] = this->requestedVertexLayout.Components[attr];
            }
        }
        // FIXME: skinned vertex components
    }

    // export vertices, all aiScene meshes go into a single vertex buffer
    int dstIndex = 0;
    this->mesh.VertexBuffer.Setup(layout, allNumVertices);
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        const int numVerts = curMesh->mNumVertices;
        assert((dstIndex + numVerts) <= allNumVertices);
        if (layout.HasAttr(VertexAttr::Position)) {
            this->mesh.VertexBuffer.Write(VertexAttr::Position, dstIndex, numVerts, (const float*)curMesh->mVertices, 3, 3);
        }
        if (layout.HasAttr(VertexAttr::Normal)) {
            assert(curMesh->HasNormals());
            this->mesh.VertexBuffer.Write(VertexAttr::Normal, dstIndex, numVerts, (const float*)curMesh->mNormals, 3, 3);
        }
        if (layout.HasAttr(VertexAttr::Tangent)) {
            assert(curMesh->HasTangentsAndBitangents());
            this->mesh.VertexBuffer.Write(VertexAttr::Tangent, dstIndex, numVerts, (const float*)curMesh->mTangents, 3, 3);
        }
        if (layout.HasAttr(VertexAttr::Binormal)) {
            assert(curMesh->HasTangentsAndBitangents());
            this->mesh.VertexBuffer.Write(VertexAttr::Binormal, dstIndex, numVerts, (const float*)curMesh->mBitangents, 3, 3);
        }
        for (int i = 0; i < 4; i++) {
            const VertexAttr::Code attr = (VertexAttr::Code)(VertexAttr::TexCoord0 + i);
            if (layout.HasAttr(attr)) {
                assert(curMesh->HasTextureCoords(i));
                this->mesh.VertexBuffer.Write(attr, dstIndex, numVerts, (const float*)curMesh->mTextureCoords[i], 3, 3);
            }
        }
        for (int i = 0; i < 2; i++) {
            const VertexAttr::Code attr = (VertexAttr::Code)(VertexAttr::Color0 + i);
            if (layout.HasAttr(attr)) {
                assert(curMesh->HasVertexColors(i));
                this->mesh.VertexBuffer.Write(attr, dstIndex, numVerts, (const float*)curMesh->mColors[i], 4, 4);
            }
        }
        // FIXME: skinned vertices components
        dstIndex += numVerts;
        assert(dstIndex <= allNumVertices);
    }
    return true;
}

//------------------------------------------------------------------------------
bool
ModelExporter::exportIndices() {
    assert(nullptr != this->scene);
    assert(nullptr != this->scene->mMeshes);
    assert(!this->mesh.IndexBuffer.IsValid());

    // compute overall number of indices
    int allNumIndices = 0;
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        for (unsigned int faceIndex = 0; faceIndex < curMesh->mNumFaces; faceIndex++) {
            const aiFace& curFace = curMesh->mFaces[faceIndex];
            if (curFace.mNumIndices != 3) {
                Log::Fatal("Encountered non-triangular face!");
            }
            allNumIndices += curFace.mNumIndices;
        }
    }

    // export indices
    int baseVertexIndex = 0;
    int startIndex = 0;
    this->mesh.IndexBuffer.Setup(this->indexSize, allNumIndices);
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        for (unsigned int faceIndex = 0; faceIndex < curMesh->mNumFaces; faceIndex++) {
            const aiFace& curFace = curMesh->mFaces[faceIndex];
            assert(curFace.mIndices);
            this->mesh.IndexBuffer.Write(startIndex, curFace.mIndices, curFace.mNumIndices, baseVertexIndex);
            startIndex += curFace.mNumIndices;
            assert(startIndex <= allNumIndices);
        }
        baseVertexIndex += curMesh->mNumVertices;
    }
    return true;
}

//------------------------------------------------------------------------------
bool
ModelExporter::exportPrimGroups() {
    assert(nullptr != this->scene);
    assert(nullptr != this->scene->mMeshes);
    assert(this->mesh.PrimGroups.empty());

    int curBaseElement = 0;
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        PrimitiveGroup primGroup;
        primGroup.Type = PrimitiveGroup::Triangles;
        primGroup.BaseElement = curBaseElement;
        primGroup.NumElements = curMesh->mNumFaces * 3;
        this->mesh.PrimGroups.push_back(primGroup);
        curBaseElement += primGroup.NumElements;
    }
    return true;
}

//------------------------------------------------------------------------------
const aiScene*
ModelExporter::GetScene() const {
    assert(this->scene);
    return this->scene;
}

//------------------------------------------------------------------------------
const Mesh&
ModelExporter::GetMesh() const {
    return this->mesh;
}

} // namespace OryolTools