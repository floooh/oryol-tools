//------------------------------------------------------------------------------
//  Config.cc
//------------------------------------------------------------------------------
#include "Config.h"
#include "ExportUtil/Log.h"
#include "assimp/postprocess.h"

namespace OryolTools {

//------------------------------------------------------------------------------
bool
Config::Load(const std::string& path) {
    try {
        this->config = cpptoml::parse_file(path);
    }
    catch (const cpptoml::parse_exception& e) {
        Log::Warn("Config::Load(): %s\n", e.what());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
unsigned int
Config::GetAiProcessFlags() const {
    unsigned int result = 0;
    if (this->config.contains("aiProcess")) {
        auto flags = this->config.get("aiProcess");
        for (const auto& item : flags->as_array()->get()) {
            const std::string val = item->as<std::string>()->get();
            if (val == "CalcTangentSpace") {
                result |= aiProcess_CalcTangentSpace;
            }
            else if (val == "JoinIdenticalVertices") {
                result |= aiProcess_JoinIdenticalVertices;
            }
            else if (val == "MakeLeftHanded") {
                result |= aiProcess_MakeLeftHanded;
            }
            else if (val == "Triangulate") {
                result |= aiProcess_Triangulate;
            }
            else if (val == "RemoveComponent") {
                result |= aiProcess_RemoveComponent;
            }
            else if (val == "GenNormals") {
                result |= aiProcess_GenNormals;
            }
            else if (val == "GenSmoothNormals") {
                result |= aiProcess_GenSmoothNormals;
            }
            else if (val == "SplitLargeMeshes") {
                result |= aiProcess_SplitLargeMeshes;
            }
            else if (val == "PreTransformVertices") {
                result |= aiProcess_PreTransformVertices;
            }
            else if (val == "LimitBoneWeights") {
                result |= aiProcess_LimitBoneWeights;
            }
            else if (val == "ValidateDataStructure") {
                result |= aiProcess_ValidateDataStructure;
            }
            else if (val == "ImproveCacheLocality") {
                result |= aiProcess_ImproveCacheLocality;
            }
            else if (val == "RemoveRedundantMaterials") {
                result |= aiProcess_RemoveRedundantMaterials;
            }
            else if (val == "FixInfacingNormals") {
                result |= aiProcess_FixInfacingNormals;
            }
            else if (val == "SortByPType") {
                result |= aiProcess_SortByPType;
            }
            else if (val == "FindDegenerates") {
                result |= aiProcess_FindDegenerates;
            }
            else if (val == "FindInvalidData") {
                result |= aiProcess_FindInvalidData;
            }
            else if (val == "GenUVCoords") {
                result |= aiProcess_GenUVCoords;
            }
            else if (val == "TransformUVCoords") {
                result |= aiProcess_TransformUVCoords;
            }
            else if (val == "FindInstances") {
                result |= aiProcess_FindInstances;
            }
            else if (val == "OptimizeMeshes") {
                result |= aiProcess_OptimizeMeshes;
            }
            else if (val == "OptimizeGraph") {
                result |= aiProcess_OptimizeGraph;
            }
            else if (val == "FlipUVs") {
                result |= aiProcess_FlipUVs;
            }
            else if (val == "FlipWindingOrder") {
                result |= aiProcess_FlipWindingOrder;
            }
            else if (val == "SplitByBoneCount") {
                result |= aiProcess_SplitByBoneCount;
            }
            else if (val == "Debone") {
                result |= aiProcess_Debone;
            }
            else {
                Log::Fatal("Invalid aiProcess flag: '%s'\n", val.c_str());
            }
        }
    }
    else {
        Log::Warn("no aiProcess entry in config!");
    }
    return result;
}

} // namesapce OryolTools