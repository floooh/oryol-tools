#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTool::Mesh
    @brief wraps an exported mesh (vertex buffer, index buffer, primitive groups)
*/
#include "ExportUtil/VertexBuffer.h"
#include "ExportUtil/IndexBuffer.h"
#include "ExportUtil/PrimitiveGroup.h"
#include <vector>

namespace OryolTools {

class Mesh {
public:
    /// embedded vertex buffer
    class VertexBuffer VertexBuffer;
    /// embedded index buffer
    class IndexBuffer IndexBuffer;
    /// primitive groups
    std::vector<PrimitiveGroup> PrimGroups;
};

} // namespace OryolTool