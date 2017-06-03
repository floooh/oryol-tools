#pragma once
//------------------------------------------------------------------------------
/**
    @class IRep
    @brief intermediate 3D model representation
*/
#include <string>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

struct IRep {
    struct VertexAttr {
        enum Enum {
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
            Invalid,
        };
    };
    struct VertexFormat {
        enum Enum {
            Float,
            Float2,
            Float3,
            Float4,
            Invalid,
        };
    };
    struct VertexComponent {
        VertexAttr::Enum Attr = VertexAttr::Invalid;
        VertexFormat::Enum Format = VertexFormat::Invalid;
        int Offset = 0;
    };

    struct PropType { 
        enum Enum {
            Float, 
            Float2, 
            Float3, 
            Float4, 
            Invalid 
        };
    };

    struct ValueProperty {
        std::string Name;
        PropType::Enum Type = PropType::Invalid;
        glm::vec4 Value; 
    };
    struct TextureProperty {
        std::string Name;
        std::string Location;
    };
    struct Material {
        std::string Name;
        std::string Shader;
        std::vector<ValueProperty> Values;
        std::vector<TextureProperty> Textures;
    };
    struct Mesh {
        uint32_t BaseVertex = 0;
        uint32_t NumVertices = 0;
        uint32_t BaseIndex = 0;
        uint32_t NumIndices = 0;
        uint32_t Material = 0;
        glm::vec3 Size;
    };
    struct Bone {
        std::string Name;
        int32_t Parent = -1;
        glm::vec3 Position;
        glm::vec3 Scaling;
        glm::vec4 Rotation;
    };
    struct Node {
        std::string Name;
        int32_t Parent = -1;
        std::vector<uint32_t> Meshes;
        glm::vec3 Position;
        glm::vec3 Scaling;
        glm::vec4 Rotation;
    };
    struct KeyType {
        enum Enum {
            Float,
            Float2,
            Float3,
            Float4,
            Quaternion,
            Invalid
        };
    };
    struct KeyComponent {
        KeyType::Enum Type = KeyType::Invalid;
    };
    struct AnimCurve {
        int32_t KeyOffset = -1;     // float index into Keys array, -1 if curve is static
        glm::vec4 StaticKey;
    };
    struct AnimClip {
        std::string Name;
        float KeyDuration = 0.0f;
        std::vector<AnimCurve> Curves;
    };

    std::vector<VertexComponent> VertexComponents;
    std::vector<Material> Materials;
    std::vector<Mesh> Meshes;
    std::vector<Bone> Bones;
    std::vector<Node> Nodes;
    std::vector<KeyComponent> KeyComponents;
    std::vector<AnimClip> AnimClips;
    std::vector<float> Vertices;
    std::vector<uint16_t> Indices;
    std::vector<float> Keys;
    
    int MaterialIndex(const std::string& name) const {
        for (int i = 0; i < int(Materials.size()); i++) {
            if (name == this->Materials[i].Name) {
                return i;
            }
        }
        return -1;
    };
};