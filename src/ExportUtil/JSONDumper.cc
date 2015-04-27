//------------------------------------------------------------------------------
//  JSONDumper.cc
//------------------------------------------------------------------------------
#include "JSONDumper.h"
#include "assimp/scene.h"
#include "cJSON.h"
#include <cassert>
#include <cstdlib>

namespace OryolTools {

//------------------------------------------------------------------------------
std::string
JSONDumper::Dump(const aiScene* scene, const std::string& srcFilePath) {
    assert(scene);

    cJSON* jsonRoot = cJSON_CreateObject();
    dumpGlobal(jsonRoot, scene, srcFilePath);
    dumpMeshes(jsonRoot, scene);
    dumpMaterials(jsonRoot, scene);
    dumpNodes(jsonRoot, scene, nullptr);

    char* rawStr = cJSON_Print(jsonRoot);
    std::string jsonStr(rawStr);
    std::free(rawStr);
    cJSON_Delete(jsonRoot);
    return jsonStr;
}

//------------------------------------------------------------------------------
void
JSONDumper::dumpGlobal(cJSON* jsonNode, const aiScene* scene, const std::string& srcFilePath) {
    cJSON_AddItemToObject(jsonNode, "path", cJSON_CreateString(srcFilePath.c_str()));
    cJSON* jsonFlags = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "flags", jsonFlags);
    unsigned int flags = scene->mFlags;
    if (flags & AI_SCENE_FLAGS_INCOMPLETE) {
        cJSON_AddItemToArray(jsonFlags, cJSON_CreateString("incomplete"));
    }
    if (flags & AI_SCENE_FLAGS_VALIDATED) {
        cJSON_AddItemToArray(jsonFlags, cJSON_CreateString("validated"));
    }
    if (flags & AI_SCENE_FLAGS_VALIDATION_WARNING) {
        cJSON_AddItemToArray(jsonFlags, cJSON_CreateString("validation_warning"));
    }
    if (flags & AI_SCENE_FLAGS_NON_VERBOSE_FORMAT) {
        cJSON_AddItemToArray(jsonFlags, cJSON_CreateString("non_verbose_format"));
    }
    if (flags & AI_SCENE_FLAGS_TERRAIN) {
        cJSON_AddItemToArray(jsonFlags, cJSON_CreateString("terrain"));
    }
}

//------------------------------------------------------------------------------
void
JSONDumper::dumpMeshes(cJSON* jsonNode, const aiScene* scene) {
    cJSON* jsonMeshes = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "meshes", jsonMeshes);
    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        cJSON* jsonMesh = cJSON_CreateObject();
        cJSON_AddItemToArray(jsonMeshes, jsonMesh);

        cJSON_AddItemToObject(jsonMesh, "name", cJSON_CreateString(mesh->mName.C_Str()));
        cJSON_AddItemToObject(jsonMesh, "num_vertices", cJSON_CreateNumber(mesh->mNumVertices));
        cJSON_AddItemToObject(jsonMesh, "num_faces", cJSON_CreateNumber(mesh->mNumFaces));
        cJSON_AddItemToObject(jsonMesh, "num_bones", cJSON_CreateNumber(mesh->mNumBones));
        cJSON_AddItemToObject(jsonMesh, "material_index", cJSON_CreateNumber(mesh->mMaterialIndex));
        cJSON_AddItemToObject(jsonMesh, "num_color_channels", cJSON_CreateNumber(mesh->GetNumColorChannels()));
        cJSON_AddItemToObject(jsonMesh, "num_uv_channels", cJSON_CreateNumber(mesh->GetNumUVChannels()));
        cJSON_AddItemToObject(jsonMesh, "has_normals", cJSON_CreateBool(mesh->HasNormals()));
        cJSON_AddItemToObject(jsonMesh, "has_tangents_and_binormals", cJSON_CreateBool(mesh->HasTangentsAndBitangents()));
        cJSON* jsonPrimTypes = cJSON_CreateArray();
        if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT) cJSON_AddItemToArray(jsonPrimTypes, cJSON_CreateString("point"));
        if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE) cJSON_AddItemToArray(jsonPrimTypes, cJSON_CreateString("line"));
        if (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) cJSON_AddItemToArray(jsonPrimTypes, cJSON_CreateString("triangle"));
        if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON) cJSON_AddItemToArray(jsonPrimTypes, cJSON_CreateString("polygon"));
        cJSON_AddItemToObject(jsonMesh, "primitive_types", jsonPrimTypes);
        cJSON* jsonNumUvComps = cJSON_CreateArray();
        for (unsigned int uvChnIndex = 0; uvChnIndex < mesh->GetNumUVChannels(); uvChnIndex++) {
            cJSON_AddItemToArray(jsonNumUvComps, cJSON_CreateNumber(mesh->mNumUVComponents[uvChnIndex]));
        }
        cJSON_AddItemToObject(jsonMesh, "num_uv_components", jsonNumUvComps);
    }
}

//------------------------------------------------------------------------------
void
JSONDumper::dumpMaterials(cJSON* jsonNode, const aiScene* scene) {
    cJSON* jsonMaterials = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "materials", jsonMaterials);
    for (unsigned int matIndex = 0; matIndex < scene->mNumMaterials; matIndex++) {
//        const aiMaterial* mat = scene->mMaterials[matIndex];
        cJSON* jsonMat = cJSON_CreateObject();
        cJSON_AddItemToArray(jsonMaterials, jsonMat);
        cJSON* jsonProps = cJSON_CreateArray();
        cJSON_AddItemToObject(jsonMat, "properties", jsonProps);

        // FIXME...
        /*
        for (unsigned int propIndex = 0; propIndex < mat->mNumProperties; propIndex++) {
            
        }
        */
    }
}

//------------------------------------------------------------------------------
void
JSONDumper::dumpNodes(cJSON* jsonNode, const aiScene* scene, const aiNode* node) {

    // root node?
    if (nullptr == node) {
        node = scene->mRootNode;
        cJSON* rootNode = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonNode, "root_node", rootNode);
        jsonNode = rootNode;
    }

    cJSON_AddItemToObject(jsonNode, "name", cJSON_CreateString(node->mName.C_Str()));
    if (node->mNumChildren > 0) {
        cJSON* jsonChildArray = cJSON_CreateArray();
        cJSON_AddItemToObject(jsonNode, "nodes", jsonChildArray);
        for (unsigned int childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
            const aiNode* childNode = node->mChildren[childIndex];
            cJSON* jsonChildNode = cJSON_CreateObject();
            cJSON_AddItemToArray(jsonChildArray, jsonChildNode);
            dumpNodes(jsonChildNode, scene, childNode);
        }
    }
}

} // namespace OryolTools