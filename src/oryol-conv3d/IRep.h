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
            Float = 0, 
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
        static int NumFloats(PropType::Enum t) {
            return (t == Invalid) ? 0 : t+1;
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
    };
    struct Bone {
        std::string Name;
        int32_t Parent = -1;
        glm::vec3 Translate = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Scale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec4 Rotate = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    };
    struct Node {
        std::string Name;
        int32_t Parent = -1;
        std::vector<Mesh> Meshes;
        glm::vec3 Translate = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Scale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec4 Rotate = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
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
        static int ByteSize(Enum t) {
            switch (t) {
                case Float: return 4;
                case Float2: return 8;
                case Float3: return 12;
                case Float4: return 16;
                case Quaternion: return 16;
                default: return 0;
            }
        }
        static int NumComponents(Enum t) {
            switch (t) {
                case Float: return 1;
                case Float2: return 2;
                case Float3: return 3;
                case Float4: return 4;
                case Quaternion: return 4;
                default: return 0;
            }
        }
    };
    struct AnimCurve {
        bool IsStatic = false;
        KeyType::Enum Type = KeyType::Invalid;
        glm::vec4 StaticKey;
        glm::vec4 Magnitude;
        std::vector<glm::vec4> Keys;
    };
    struct AnimClip {
        std::string Name;
        float KeyDuration = 0.0f;
        std::vector<AnimCurve> Curves;
    };

    std::vector<VertexComponent> VertexComponents;
    std::vector<Material> Materials;
    std::vector<Bone> Bones;
    std::vector<Node> Nodes;
    std::vector<AnimClip> AnimClips;
    glm::vec3 VertexMagnitude;
    std::vector<float> VertexData;
    std::vector<uint16_t> IndexData;

    /// compute the vertex position magnitude
    void ComputeVertexMagnitude();
    /// compute the anim curve magnitude values (max(abs(key)) over all keys)
    void ComputeCurveMagnitudes();
    /// computed getters
    bool HasVertexAttr(VertexAttr::Code attr) const;
    int MaterialIndex(const std::string& name) const;
    int VertexStrideBytes() const;
    int NumMeshVertices() const;
    int NumValueProps() const;
    int NumPropValues() const;
    int NumTextureProps() const;
    int NumMeshes() const;
    int NumAnimCurves() const;
    int NumAnimCurvesPerClip() const;
    int AnimClipKeyStride(int clipIndex) const;
    int AnimClipLength(int clipIndex) const;
    int AnimKeyDataSize() const;
    int AnimKeyOffset(int clipIndex, int curveIndex) const;
    std::vector<std::string> NodeNames() const;
    std::vector<std::string> ClipNames() const;
};
