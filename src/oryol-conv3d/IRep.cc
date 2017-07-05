//------------------------------------------------------------------------------
//  IRep.cc
//------------------------------------------------------------------------------
#include "IRep.h"
#include <glm/glm.hpp>

//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
bool
IRep::HasVertexAttr(VertexAttr::Code attr) const {
    for (const auto& comp : this->VertexComponents) {
        if (comp.Attr == attr) {
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------------------------
int
IRep::MaterialIndex(const std::string& name) const {
    for (size_t i = 0; i < this->Materials.size(); i++) {
        if (name == this->Materials[i].Name) {
            return (int)i;
        }
    }
    return -1;
}

//-------------------------------------------------------------------------
int
IRep::VertexStrideBytes() const {
    // number of bytes(!) from one vertex to next
    int stride = 0;
    for (const auto& comp : this->VertexComponents) {
        stride += VertexFormat::ByteSize(comp.Format);
    }
    return stride;
}

//-------------------------------------------------------------------------
int
IRep::NumMeshVertices() const {
    int num = 0;
    for (const auto& node : this->Nodes) {
        for (const auto& mesh : node.Meshes) {
            num += mesh.NumVertices;
        }
    }
    return num;
}

//-------------------------------------------------------------------------
int
IRep::NumValueProps() const {
    size_t num = 0;
    for (const auto& mat : this->Materials) {
        num += mat.Values.size();
    }
    return (int)num;
}

//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
int
IRep::NumTextureProps() const {
    size_t num = 0;
    for (const auto& mat : this->Materials) {
        num += mat.Textures.size();
    }
    return (int)num;
}

//-------------------------------------------------------------------------
int
IRep::NumMeshes() const {
    size_t num = 0;
    for (const auto& node : this->Nodes) {
        num += node.Meshes.size();
    }
    return num;
}

//-------------------------------------------------------------------------
int
IRep::NumAnimCurves() const {
    size_t num = 0;
    for (const auto& clip : this->AnimClips) {
        num += clip.Curves.size();
    }
    return (int)num;
}

//-------------------------------------------------------------------------
int
IRep::NumAnimCurvesPerClip() const {
    if (this->AnimClips.empty()) {
        return 0;
    }
    else {
        return this->AnimClips[0].Curves.size();
    }
}

//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
int
IRep::AnimKeyDataSize() const {
    int animKeyDataSize = 0;
    for (int clipIndex = 0; clipIndex < int(this->AnimClips.size()); clipIndex++) {
        animKeyDataSize += this->AnimClipKeyStride(clipIndex) * this->AnimClipLength(clipIndex);
    }
    return animKeyDataSize;
}

//-------------------------------------------------------------------------
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

