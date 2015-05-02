#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::JSONMeshDumper
    @brief dump mesh content as json for debugging
*/
#include <string>
#include "ExportUtil/Mesh.h"

struct cJSON;

namespace OryolTools {

class JSONMeshDumper {
public:
    /// dump mesh object to string
    static std::string Dump(const Mesh& mesh);

private:
    /// dump header info
    static void dumpHeader(cJSON* jsonNode, const Mesh& mesh);
    /// dump vertices to json
    static void dumpVertices(cJSON* jsonNode, const Mesh& mesh);
    /// dump indices to json
    static void dumpIndices(cJSON* jsonNode, const Mesh& mesh);
};

} // namespace OryolTools
