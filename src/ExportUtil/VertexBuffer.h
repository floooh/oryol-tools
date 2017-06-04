#pragma once
//------------------------------------------------------------------------------
/**
    @class VertexBuffer
    @brief holds exported vertex data
*/
#include "ExportUtil/Vertex.h"
#include <cstdint>

class VertexBuffer {
public:
    /// destructor
    ~VertexBuffer();

    /// setup the object
    void Setup(const VertexLayout& layout, int numVertices);
    /// discard the object
    void Discard();
    /// return true if object has been setup
    bool IsValid() const;

    /// write a vertex component stream, return next vertex index
    void Write(VertexAttr::Code attr, int startVertexIndex, int numVertices, const float* input, int numInputComps, int inputStride);

    /// get the vertex layout
    const VertexLayout& GetVertexLayout() const;
    /// get the number of vertices
    int GetNumVertices() const;
    /// get pointer to vertex data
    const std::uint8_t* GetDataPointer() const;
    /// get size of vertex data in bytes
    int GetDataSize() const;

private:
    /// internal specialized vertex writer method
    template<VertexFormat::Code FORMAT> void write(VertexAttr::Code attr, std::uint8_t* dstPtr, int numVerts, const float* input, int numInputComps, int inputStide);

    VertexLayout layout;
    int allNumVertices = 0;
    std::uint8_t* buffer = nullptr;
};
