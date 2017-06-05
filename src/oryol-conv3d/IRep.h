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
#include "ExportUtil/Vertex.h"

struct IRep {
    struct VertexComponent {
        VertexAttr::Code Attr = VertexAttr::Invalid;
        VertexFormat::Code Format = VertexFormat::Invalid;
        int Offset = 0; // in bytes!
    };

    struct PropType { 
        enum Enum {
            Float, 
            Float2, 
            Float3, 
            Float4, 
            Invalid 
        };
        static const char* ToString(PropType::Enum t) {
            switch (t) {
                case Float: return "Float";
                case Float2: return "Float2";
                case Float3: return "Float3";
                case Float4: return "Float4";
                default: return "Invalid";
            }
        }
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
        uint32_t FirstVertex = 0;
        uint32_t NumVertices = 0;
        uint32_t FirstIndex = 0;
        uint32_t NumIndices = 0;
        uint32_t Material = 0;
        glm::vec3 Size;
    };
    struct Bone {
        std::string Name;
        int32_t Parent = -1;
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Scaling = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec4 Rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    };
    struct Node {
        std::string Name;
        int32_t Parent = -1;
        std::vector<uint32_t> Meshes;
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Scaling = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec4 Rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
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
        static const char* ToString(Enum t) {
            switch (t) {
                case Float: return "Float";
                case Float2: return "Float2";
                case Float3: return "Float3";
                case Float4: return "Float4";
                case Quaternion: return "Quaternion";
                default: return "Invalid";
            }
        }
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
    std::vector<float> VertexData;
    std::vector<uint16_t> IndexData;
    std::vector<float> KeyData;
    
    int MaterialIndex(const std::string& name) const {
        for (int i = 0; i < int(Materials.size()); i++) {
            if (name == this->Materials[i].Name) {
                return i;
            }
        }
        return -1;
    };
};
