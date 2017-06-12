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
        glm::vec3 Size;
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
    };
    struct AnimCurve {
        bool IsStatic = false;
        KeyType::Enum Type = KeyType::Invalid;
        glm::vec4 StaticKey;
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
    std::vector<float> VertexData;
    std::vector<uint16_t> IndexData;

    //-------------------------------------------------------------------------
    bool HasVertexAttr(VertexAttr::Code attr) const {
        for (const auto& comp : this->VertexComponents) {
            if (comp.Attr == attr) {
                return true;
            }
        }
        return false;
    };
    //-------------------------------------------------------------------------
    int MaterialIndex(const std::string& name) const {
        for (size_t i = 0; i < this->Materials.size(); i++) {
            if (name == this->Materials[i].Name) {
                return (int)i;
            }
        }
        return -1;
    };
    //-------------------------------------------------------------------------
    int VertexStrideBytes() const {
        // number of bytes(!) from one vertex to next
        int stride = 0;
        for (const auto& comp : this->VertexComponents) {
            stride += VertexFormat::ByteSize(comp.Format);
        }
        return stride;
    }
    //-------------------------------------------------------------------------
    int NumMeshVertices() const {
        int num = 0;
        for (const auto& node : this->Nodes) {
            for (const auto& mesh : node.Meshes) {
                num += mesh.NumVertices;
            }
        }
        return num;
    }
    //-------------------------------------------------------------------------
    int NumValueProps() const {
        size_t num = 0;
        for (const auto& mat : this->Materials) {
            num += mat.Values.size();
        }
        return (int)num;
    };
    //-------------------------------------------------------------------------
    int NumPropValues() const {
        size_t num = 0;
        for (const auto& mat : this->Materials) {
            for (const auto& prop : mat.Values) {
                num += PropType::NumFloats(prop.Type); 
            }
        }
        return num;
    };
    //-------------------------------------------------------------------------
    int NumTextureProps() const {
        size_t num = 0;
        for (const auto& mat : this->Materials) {
            num += mat.Textures.size();
        }
        return (int)num;
    }
    //-------------------------------------------------------------------------
    int NumMeshes() const {
        size_t num = 0;
        for (const auto& node : this->Nodes) {
            num += node.Meshes.size();
        }   
        return num;
    }
    //-------------------------------------------------------------------------
    int NumAnimCurves() const {
        size_t num = 0;
        for (const auto& clip : this->AnimClips) {
            num += clip.Curves.size();
        }
        return (int)num;
    }
    //-------------------------------------------------------------------------
    int NumAnimCurvesPerClip() const {
        if (this->AnimClips.empty()) {
            return 0;
        }
        else {
            return this->AnimClips[0].Curves.size();
        }
    }
    //-------------------------------------------------------------------------
    int AnimClipKeyStride(int clipIndex) const {
        int stride = 0;
        const auto& clip = this->AnimClips[clipIndex];
        for (const auto& curve : clip.Curves) {
            if (!curve.IsStatic) {
                stride += KeyType::ByteSize(curve.Type);
            }
        }
        return stride;
    }
    //-------------------------------------------------------------------------
    int AnimClipLength(int clipIndex) const {
        for (const auto& curve : this->AnimClips[clipIndex].Curves) {
            // all curves in the clip either have no keys, or the same number of keys
            if (!curve.Keys.empty()) {
                return curve.Keys.size();
            }
        }
        return 0;
    }
    //-------------------------------------------------------------------------
    int AnimKeyDataSize() const {
        int animKeyDataSize = 0;
        for (int clipIndex = 0; clipIndex < int(this->AnimClips.size()); clipIndex++) {
            animKeyDataSize += this->AnimClipKeyStride(clipIndex) * this->AnimClipLength(clipIndex);
        }
        return animKeyDataSize;
    }
    //-------------------------------------------------------------------------
    int AnimKeyOffset(int clipIndex, int curveIndex) const {
        int keyOffset = 0;
        for (int i = 0; i < clipIndex; i++) {
            keyOffset += this->AnimClipLength(i) * this->AnimClipKeyStride(i);
        }
        const auto& clip = this->AnimClips[clipIndex];
        for (int i = 0; i < curveIndex; i++) {
            const auto& curve = clip.Curves[i];
            if (!curve.IsStatic) {
                keyOffset += KeyType::ByteSize(curve.Type);
            }
        }
        return keyOffset;
    }
};
