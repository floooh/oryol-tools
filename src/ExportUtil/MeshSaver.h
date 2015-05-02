#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::MeshSaver
    @brief save mesh data to binary file
    
    File format:
    
    uint32_t magic = 'OMSH';
    uint32_t numVertices;
    uint32_t vertexSize;    // size of one vertex in bytes
    uint32_t numIndices;
    uint32_t indexSize = 2 or 4;
    uint32_t numVertexComponents;
    uint32_t numPrimitiveGroups;
    struct {
        uint32_t attr = {
            Position: 0,
            Normal: 1,
            TexCoord0: 2,
            TexCoord1: 3,
            TexCoord2: 4,
            TexCoord3: 5,
            Tangent: 6,
            Binormal: 7,
            Weights: 8,
            Indices: 9,
            Color0: 10
            Color1: 11
        };
        uint32 format = {
            Float: 0,
            Float2: 1,
            Float3: 2,
            Float4: 3,
            Byte4: 4,
            Byte4N: 5,
            UByte4: 6,
            UByte4N: 7,
            Short2: 8,
            Short2N: 9
            Short4: 10,
            Short4N: 11
        };
    } vertexComponents[numVertexComponents];
    struct {
        uint32_t type = {
            Points: 0,
            Lines: 1,
            LineLoop: 2,
            LineStrip: 3,
            Triangles: 4,
            TriangleStrip: 5,
            TriangleFan: 6
        };
        uint32_t baseElement;
        uint32_t numElements;
    } primitiveGroups[numPrimitiveGroups];
    uint8_t vertexData[numVertices * vertexSize];
    uint8_t indexData[numIndices * indexSize];
    [optional 2 bytes of padding if odd number of 16-bit indices]
*/
#include <cstdio>
#include "ExportUtil/Mesh.h"

namespace OryolTools {

class MeshSaver {
public:
    /// save mesh to FILE*, return number of bytes written
    static size_t Save(const Mesh& mesh, FILE* fp);

private:
    /// helper to write an uint32_t value
    static size_t write32(FILE* fp, std::uint32_t val);
};

} // namespace OryolTools
