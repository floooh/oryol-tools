#pragma once
//------------------------------------------------------------------------------
/**
    @class NVX2Loader
    @brief append-load NVX2 mesh files
*/
#include <string>
#include <vector>
#include <map>
#include "ExportUtil/Vertex.h"

struct NVX2Loader {
    struct PrimGroup {
        int FirstVertex = 0;
        int NumVertices = 0;
        int FirstIndex = 0;
        int NumIndices = 0;
    };
    struct Component {
        VertexAttr::Code Attr = VertexAttr::Invalid; 
        VertexFormat::Code SrcFormat = VertexFormat::Invalid;
        VertexFormat::Code DstFormat = VertexFormat::Invalid;
        float Scale = 1.0f;
        float Bias = 0.0f;
        int SrcOffset = 0;  // in number of bytes
        int DstOffset = 0;  // in number of bytes
        
        Component(VertexAttr::Code attr, VertexFormat::Code src, VertexFormat::Code dst, float s=1.0f, float b=0.0f):
            Attr(attr), SrcFormat(src), DstFormat(dst), Scale(s), Bias(b) { };
        bool operator==(const Component& rhs) const {
            return (this->Attr == rhs.Attr) && (this->SrcFormat == rhs.SrcFormat) && (this->DstFormat == rhs.DstFormat) &&
                   (this->SrcOffset == rhs.SrcOffset) && (this->DstOffset == rhs.DstOffset);
        };
        bool operator!=(const Component& rhs) const {
            return !this->operator==(rhs);
        };
    };
    struct Mesh {
        std::string Name;
        int NumVertices = 0;
        int NumIndices = 0;
        int SrcStride = 0;      // in number of bytes
        int DstStride = 0;      // in number of bytes
        std::vector<Component> Components;
        std::vector<PrimGroup> PrimGroups;
        std::vector<float> VertexData;
        std::vector<uint16_t> IndexData;
    };
    
    /// in: the expected vertex layout, only the attributes matter
    VertexLayout Layout;

    /// append-load an NVX2 mesh file
    void Load(const std::string& nvx2AssetName, const std::string& n3AssetDir);
    /// clear the loader
    void Clear();
    /// return true if a mesh exists in the loader
    bool HasMesh(const std::string& nvx2AssetName) const;
    /// get mesh by its asset name
    const Mesh& MeshByName(const std::string& nvx2AssetName) const;
    /// check that all meshes have identical vertex layouts
    void ValidateVertexLayouts() const;
    /// returns a primitive group with absolute base indices
    PrimGroup AbsPrimGroup(const std::string& nvx2AssetName, int localPrimGroupIndex) const;
    /// return overall number of vertices
    int NumVertices() const;
    /// return overall number of indices
    int NumIndices() const;
    /// return the vertex stride in number of bytes
    int VertexStride() const;

    std::vector<Mesh> Meshes;

    enum N2VertexComponent
    {
        N2Coord        = (1<<0),      // 3 floats
        N2Normal       = (1<<1),      // 3 floats
        N2NormalUB4N   = (1<<2),      // 4 unsigned bytes, normalized
        N2Uv0          = (1<<3),      // 2 floats
        N2Uv0S2        = (1<<4),      // 2 shorts, 4.12 fixed point
        N2Uv1          = (1<<5),      // 2 floats
        N2Uv1S2        = (1<<6),      // 2 shorts, 4.12 fixed point
        N2Uv2          = (1<<7),      // 2 floats
        N2Uv2S2        = (1<<8),      // 2 shorts, 4.12 fixed point
        N2Uv3          = (1<<9),      // 2 floats
        N2Uv3S2        = (1<<10),     // 2 shorts, 4.12 fixed point
        N2Color        = (1<<11),     // 4 floats
        N2ColorUB4N    = (1<<12),     // 4 unsigned bytes, normalized
        N2Tangent      = (1<<13),     // 3 floats
        N2TangentUB4N  = (1<<14),     // 4 unsigned bytes, normalized
        N2Binormal     = (1<<15),     // 3 floats
        N2BinormalUB4N = (1<<16),     // 4 unsigned bytes, normalized
        N2Weights      = (1<<17),     // 4 floats
        N2WeightsUB4N  = (1<<18),     // 4 unsigned bytes, normalized
        N2JIndices     = (1<<19),     // 4 floats
        N2JIndicesUB4  = (1<<20),     // 4 unsigned bytes

        N2NumVertexComponents = 21,
        N2AllComponents = ((1<<N2NumVertexComponents) - 1),
    };
    #pragma pack(push, 1)
    struct Nvx2Header {
        uint32_t Magic = 0;
        uint32_t NumGroups = 0;
        uint32_t NumVertices = 0;
        uint32_t VertexWidth = 0;
        uint32_t NumTriangles = 0;
        uint32_t NumEdges = 0;
        uint32_t VertexComponentMask = 0;
    };
    struct Nvx2Group {
        uint32_t FirstVertex = 0;
        uint32_t NumVertices = 0;
        uint32_t FirstTriangle = 0;
        uint32_t NumTriangles = 0;
        uint32_t FirstEgde = 0;
        uint32_t NumEdges = 0;
    };
    #pragma pack(pop)
};
