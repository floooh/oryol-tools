//------------------------------------------------------------------------------
//  IRep.cc
//------------------------------------------------------------------------------
#include "IRep.h"
#include "ExportUtil/Log.h"
#include <glm/glm.hpp>

using namespace OryolTools;

//------------------------------------------------------------------------------
void
IRep::ComputeVertexMagnitude() {
    assert((this->VertexComponents[0].Attr == VertexAttr::Position) &&
           (this->VertexComponents[0].Format == VertexFormat::Float3));
    glm::vec3 mag(0.0f);
    glm::vec3 pos(0.0f);
    const int stride = this->VertexStrideBytes() / sizeof(float);
    for (const auto& node : this->Nodes) {
        for (const auto& mesh : node.Meshes) {
            for (int i = 0; i < int(mesh.VertexData.size()); i += stride) {
                pos.x = mesh.VertexData[i + 0];
                pos.y = mesh.VertexData[i + 1];
                pos.z = mesh.VertexData[i + 2];
                mag = glm::max(mag, glm::abs(pos));
            }
        }
    }
   this->VertexMagnitude = glm::vec3(mag);
}

//------------------------------------------------------------------------------
void
IRep::ComputeCurveMagnitudes() {
    for (auto& clip : this->AnimClips) {
        for (auto& curve : clip.Curves) {
            curve.Magnitude = glm::vec4(0.0f);
            for (const auto& key : curve.Keys) {
                curve.Magnitude = glm::max(curve.Magnitude, glm::abs(key));
            }
        }
    }
}

//------------------------------------------------------------------------------
bool
IRep::HasVertexAttr(VertexAttr::Code attr) const {
    for (const auto& comp : this->VertexComponents) {
        if (comp.Attr == attr) {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
int
IRep::MaterialIndex(const std::string& name) const {
    for (size_t i = 0; i < this->Materials.size(); i++) {
        if (name == this->Materials[i].Name) {
            return (int)i;
        }
    }
    return -1;
}

//------------------------------------------------------------------------------
int
IRep::VertexStrideBytes() const {
    // number of bytes(!) from one vertex to next
    int stride = 0;
    for (const auto& comp : this->VertexComponents) {
        stride += VertexFormat::ByteSize(comp.Format);
    }
    return stride;
}

//------------------------------------------------------------------------------
int
IRep::NumVertices() const {
    int num = 0;
    const int stride = this->VertexStrideBytes() / sizeof(float);
    for (const auto& node : this->Nodes) {
        for (const auto& mesh : node.Meshes) {
            Log::FailIf((mesh.VertexData.size() % stride) != 0, "Vertex data size isn't multiple of vertex stride!\n");
            num += mesh.VertexData.size() / stride;
        }
    }
    return num;
}

//------------------------------------------------------------------------------
int
IRep::NumIndices() const {
    int num = 0;
    for (const auto& node : this->Nodes) {
        for (const auto& mesh : node.Meshes) {
            Log::FailIf((mesh.IndexData.size() % 3) != 0, "Index data size isn't multiple of 3!\n");
            num += mesh.IndexData.size();
        }
    }
    return num;
}

//------------------------------------------------------------------------------
int
IRep::NumValueProps() const {
    size_t num = 0;
    for (const auto& mat : this->Materials) {
        num += mat.Values.size();
    }
    return (int)num;
}

//------------------------------------------------------------------------------
int
IRep::NumPropValues() const {
    size_t num = 0;
    for (const auto& mat : this->Materials) {
        for (const auto& prop : mat.Values) {
            num += PropType::NumFloats(prop.Type);
        }
    }
    return num;
}

//------------------------------------------------------------------------------
int
IRep::NumTextureProps() const {
    size_t num = 0;
    for (const auto& mat : this->Materials) {
        num += mat.Textures.size();
    }
    return (int)num;
}

//------------------------------------------------------------------------------
int
IRep::NumMeshes() const {
    size_t num = 0;
    for (const auto& node : this->Nodes) {
        num += node.Meshes.size();
    }
    return num;
}

//------------------------------------------------------------------------------
int
IRep::NumAnimCurves() const {
    size_t num = 0;
    for (const auto& clip : this->AnimClips) {
        num += clip.Curves.size();
    }
    return (int)num;
}

//------------------------------------------------------------------------------
int
IRep::NumAnimCurvesPerClip() const {
    if (this->AnimClips.empty()) {
        return 0;
    }
    else {
        return this->AnimClips[0].Curves.size();
    }
}

//------------------------------------------------------------------------------
int
IRep::AnimClipKeyStride(int clipIndex) const {
    int stride = 0;
    const auto& clip = this->AnimClips[clipIndex];
    for (const auto& curve : clip.Curves) {
        if (!curve.IsStatic) {
            stride += KeyType::ByteSize(curve.Type);
        }
    }
    return stride;
}

//------------------------------------------------------------------------------
int
IRep::AnimClipLength(int clipIndex) const {
    for (const auto& curve : this->AnimClips[clipIndex].Curves) {
        // all curves in the clip either have no keys, or the same number of keys
        if (!curve.Keys.empty()) {
            return curve.Keys.size();
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
int
IRep::AnimKeyDataSize() const {
    int animKeyDataSize = 0;
    for (int clipIndex = 0; clipIndex < int(this->AnimClips.size()); clipIndex++) {
        animKeyDataSize += this->AnimClipKeyStride(clipIndex) * this->AnimClipLength(clipIndex);
    }
    return animKeyDataSize;
}

//------------------------------------------------------------------------------
int
IRep::AnimKeyOffset(int clipIndex, int curveIndex) const {
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

//------------------------------------------------------------------------------
std::vector<std::string>
IRep::NodeNames() const {
    std::vector<std::string> res;
    res.reserve(this->Nodes.size());
    for (const auto& node : this->Nodes) {
        res.push_back(node.Name);
    }
    return res;
}

//------------------------------------------------------------------------------
std::vector<std::string>
IRep::ClipNames() const {
    std::vector<std::string> res;
    res.reserve(this->AnimClips.size());
    for (const auto& clip : this->AnimClips) {
        res.push_back(clip.Name);
    }
    return res;
}
