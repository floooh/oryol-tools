#pragma once
//------------------------------------------------------------------------------
/**
    @class VertexCodec
    @brief decode and encode vertex components
*/
#include <stdint.h>
#include "ExportUtil/Vertex.h"

class VertexCodec {
public:
    /// encode from generic float vertex data
    template<VertexFormat::Code FORMAT> static uint8_t* Encode(uint8_t* dst, float scale, const float* src, int numSrcComps);
    /// decode into generic float vertex data
    template<VertexFormat::Code FORMAT> static void Decode(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps);
};
