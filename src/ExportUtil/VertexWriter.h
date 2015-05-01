#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::VertexWriter
    @brief helper class to write packed vertex components
*/
#include <cstdint>
#include "ExportUtil/Vertex.h"

namespace OryolTools {

class VertexWriter {
public:
    /// write 1D..4D vertex component
    template<VertexFormat::Code FORMAT> static void Write(std::uint8_t* dst, float scale, const float* src, int numSrcComps);
};

} // namespace OryolTools