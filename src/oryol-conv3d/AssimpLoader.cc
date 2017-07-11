//------------------------------------------------------------------------------
//  AssimpLoader.cc
//------------------------------------------------------------------------------
#include "AssimpLoader.h"
#include "ExportUtil/Log.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

using namespace OryolTools;

//------------------------------------------------------------------------------
void
AssimpLoader::Load(const std::string& path, IRep& irep) {
    Log::FailIf(path.empty(), "path to 3D file required!");

    this->importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_LIGHTS|aiComponent_CAMERAS);
    this->importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT|aiPrimitiveType_LINE);
    this->importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, (1<<16));
    const uint32_t procFlags =
        aiProcess_CalcTangentSpace|
        aiProcess_JoinIdenticalVertices|
        aiProcess_Triangulate|
        aiProcess_GenNormals|   // only if mesh doesn't already have normals
        aiProcess_ImproveCacheLocality|
        aiProcess_SplitLargeMeshes|
        aiProcess_RemoveRedundantMaterials|
        aiProcess_SortByPType|
        aiProcess_FindDegenerates|
        aiProcess_FindInvalidData|
        aiProcess_GenUVCoords|
        aiProcess_TransformUVCoords|
        aiProcess_OptimizeMeshes|
        aiProcess_OptimizeGraph|
        aiProcess_FlipWindingOrder;
    this->scene = importer.ReadFile(path, procFlags);
    Log::FailIf(!this->scene, "Failed to import file '%s' via assimp: %s\n", path.c_str(), importer.GetErrorString());

    this->toIRep(irep);
}

//------------------------------------------------------------------------------
void
AssimpLoader::toIRep(IRep& irep) {
    this->writeVertexComponents(irep);

    // FIXME
}

//------------------------------------------------------------------------------
void
AssimpLoader::writeVertexComponents(IRep& irep) {
    std::array<VertexFormat::Code, VertexAttr::Num> comps;
    comps.fill(VertexFormat::Invalid);
    for (uint32_t meshIndex = 0; meshIndex < this->scene->mNumMeshes; meshIndex++) {
        const aiMesh* msh = scene->mMeshes[meshIndex];
        if (msh->HasPositions()) {
            comps[VertexAttr::Position] = VertexFormat::Float3;
        }
        if (msh->HasNormals()) {
            comps[VertexAttr::Normal] = VertexFormat::Float3;
        }
        if (msh->HasTangentsAndBitangents()) {
            comps[VertexAttr::Tangent] = VertexFormat::Float3;
            comps[VertexAttr::Binormal] = VertexFormat::Float3;
        }
        for (uint32_t i = 0; i < 4; i++) {
            if (msh->HasTextureCoords(i)) {
                comps[VertexAttr::TexCoord0+i] = VertexFormat::Float2;
            }
        }
        for (uint32_t i = 0; i < 2; i++) {
            if (msh->HasVertexColors(i)) {
                comps[VertexAttr::Color0+i] = VertexFormat::Float4;
            }
        }
        if (msh->HasBones()) {
            comps[VertexAttr::Weights] = VertexFormat::Float4;
            comps[VertexAttr::Indices] = VertexFormat::Float4;
        }
    }
    for (int i = 0; i < VertexAttr::Num; i++) {
        if (comps[i] != VertexFormat::Invalid) {
            IRep::VertexComponent c;
            c.Attr = (VertexAttr::Code) i;
            c.Format = comps[i];
            irep.VertexComponents.push_back(c);
        }
    }
}
