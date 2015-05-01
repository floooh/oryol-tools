#pragma once
//------------------------------------------------------------------------------
/** 
    @class OryolTools::JSONDumper
    @brief dump an AssImp aiScene to a JSON-formatted string
*/
#include <string>

struct aiScene;
struct aiNode;
struct cJSON;

namespace OryolTools {

class JSONSceneDumper {
public:
    /// dump aiScene to string
    static std::string Dump(const aiScene* scene, const std::string& srcFilePath);

private:
    /// dump global scene flags
    static void dumpGlobal(cJSON* jsonNode, const aiScene* scene, const std::string& srcFilePath);
    /// dump mesh info
    static void dumpMeshes(cJSON* jsonNode, const aiScene* scene);
    /// dump material info
    static void dumpMaterials(cJSON* jsonNode, const aiScene* scene);
    /// dump nodes (top-level, or recursive)
    static void dumpNodes(cJSON* jsonNode, const aiScene* scene, const aiNode* node);
};

} // namespace OryolTools