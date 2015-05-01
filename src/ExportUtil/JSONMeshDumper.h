#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::JSONMeshDumper
    @brief dump mesh content as json for debugging
*/
#include <string>
#include "ExportUtil/Mesh.h"

namespace OryolTools {

class JSONMeshDumper {
public:
    /// dump mesh object to string
    static std::string Dump(const Mesh& mesh);
};

} // namespace OryolTools
