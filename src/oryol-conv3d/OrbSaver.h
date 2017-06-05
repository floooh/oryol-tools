#pragma once
//------------------------------------------------------------------------------
/**
    @class OrbSaver
    @brief save intermediate representation to ORB (ORyol Binary format)
*/
#include <string.h>
#include "IRep.h"

struct OrbSaver {
    /// save IRep to ORB
    void Save(const std::string& path, const IRep& irep);

    #pragma pack(push,1)
    // NOTE: offsets and sizes are in bytes, all offsets and sizes must be 4-byte multiples
    struct OrbHeader {
        uint32_t Magic;
        uint32_t VertexComponentOffset;
        uint32_t NumVertexComponents;
        uint32_t ValuePropOffset;
        uint32_t NumValueProps;
        uint32_t TexturePropOffset;
        uint32_t NumTextureProps;
        uint32_t MaterialOffset;
        uint32_t NumMaterials;
        uint32_t MeshOffset;
        uint32_t NumMeshes;
        uint32_t BoneOffset;
        uint32_t NumBones;
        uint32_t NodeOffset;
        uint32_t NumNodes;
        uint32_t AnimKeyComponentOffset;
        uint32_t NumAnimKeyComponents;
        uint32_t AnimCurveOffset;
        uint32_t NumAnimCurves;
        uint32_t AnimClipOffset;
        uint32_t NumAnimClips;
        uint32_t ValuePoolOffset;
        uint32_t ValuePoolSize;
        uint32_t VertexDataOffset;
        uint32_t VertexDataSize;
        uint32_t IndexDataOffset;
        uint32_t IndexDataSize;     // must be multiple of 4
        uint32_t AnimKeyDataOffset;
        uint32_t AnimKeyDataSize;
        uint32_t StringPoolDataOffset;
        uint32_t StringPoolDataSize;
    };
    enum class OrbVertexAttr : uint32_t {
        Invalid = 0,
        Position,
        Normal,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        Tangent,
        Binormal,
        Weights,
        Indices,
        Color0,
        Color1,
    };
    enum class OrbVertexFormat : uint32_t {
        Invalid = 0,
        Float,
        Float2,
        Float3,
        Float4,
        Byte4,
        Byte4N,
        UByte4,
        UByte4N,
        Short2,
        Short2N,
        Short4,
        Short4N
    };
    struct OrbVertexComponent {
        OrbVertexAttr Attr = OrbVertexAttr::Invalid;
        OrbVertexFormat Format = OrbVertexFormat::Invalid;
    };
    struct OrbValueProperty {
        uint32_t Name;
        uint32_t ValueIndex;        // index(!) of float value in value pool
        uint32_t NumValues;         // 1..4 (for float, vec2, vec3, vec4)
    };
    struct OrbTextureProperty {
        uint32_t Name;
        uint32_t Location;          // string pool
    };
    struct OrbMaterial {
        uint32_t Name;
        uint32_t Shader;
        uint32_t FirstValueProp;
        uint32_t NumValueProp;
        uint32_t FirstTextureProp;
        uint32_t NumTextureProps;
    };
    struct OrbMesh {
        uint32_t Material;
        uint32_t FirstVertex;
        uint32_t NumVertices;
        uint32_t FirstIndex;
        uint32_t NumIndices;
        float Size[3];
    };
    struct OrbBone {
        uint32_t Name;
        int32_t Parent;
        float PoseTranslate[3];
        float PoseScaling[3];
        float PoseRotate[4];
    };
    struct OrbNode {
        uint32_t Name;
        int32_t Parent;
        uint32_t FirstMesh;
        uint32_t NumMeshes;
        float Translate[3];
        float Scaling[3];
        float Rotate[4];
    };
    #pragma pack() 
};