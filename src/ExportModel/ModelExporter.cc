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
ModelExporter::~ModelExporter() {
    this->scene = 0;
    this->mesh.VertexBuffer.Discard();
    this->mesh.IndexBuffer.Discard();
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
bool
ModelExporter::Export(const std::string& path) {
    assert(!path.empty());
    assert(!this->mesh.VertexBuffer.IsValid());
    assert(!this->mesh.IndexBuffer.IsValid());
    assert(this->mesh.PrimGroups.empty());

    this->exportVertices();
    this->exportIndices();
    this->exportPrimGroups();

    return true;
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
    this->mesh.IndexBuffer.Setup(this->indexSize, allNumIndices);
    for (unsigned int meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* curMesh = this->scene->mMeshes[meshIndex];
        int startIndex = 0;
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

} // namespace OryolTools