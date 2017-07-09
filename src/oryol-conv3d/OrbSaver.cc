//------------------------------------------------------------------------------
//  OrbSaver.cc
//------------------------------------------------------------------------------
#include "OrbSaver.h"
#include "ExportUtil/Log.h"
#include "ExportUtil/VertexCodec.h"
#include <glm/glm.hpp>
#include <stdio.h>

using namespace OryolTools;
using namespace Oryol;

//------------------------------------------------------------------------------
uint32_t
OrbSaver::addString(const std::string& str) {
    ptrdiff_t index = std::find(strings.begin(), strings.end(), str) - strings.begin();
    if (int(index) < int(strings.size())) {
        // string already exists
        return index;
    }
    else {
        // add new string
        index = strings.size();
        strings.push_back(str);
    }
    return index;
}

//------------------------------------------------------------------------------
static OrbVertexAttr::Enum
toOrbVertexAttr(VertexAttr::Code attr) {
    switch (attr) {
        case VertexAttr::Position:      return OrbVertexAttr::Position;
        case VertexAttr::Normal:        return OrbVertexAttr::Normal;
        case VertexAttr::TexCoord0:     return OrbVertexAttr::TexCoord0;
        case VertexAttr::TexCoord1:     return OrbVertexAttr::TexCoord1;
        case VertexAttr::TexCoord2:     return OrbVertexAttr::TexCoord2;
        case VertexAttr::TexCoord3:     return OrbVertexAttr::TexCoord3;
        case VertexAttr::Tangent:       return OrbVertexAttr::Tangent;
        case VertexAttr::Binormal:      return OrbVertexAttr::Binormal;
        case VertexAttr::Weights:       return OrbVertexAttr::Weights;
        case VertexAttr::Indices:       return OrbVertexAttr::Indices;
        case VertexAttr::Color0:        return OrbVertexAttr::Color0;
        case VertexAttr::Color1:        return OrbVertexAttr::Color1;
        default:                        return OrbVertexAttr::Invalid;
    }
}

//------------------------------------------------------------------------------
static OrbVertexFormat::Enum
toOrbVertexFormat(VertexFormat::Code fmt) {
    switch (fmt) {
        case VertexFormat::Float:       return OrbVertexFormat::Float;
        case VertexFormat::Float2:      return OrbVertexFormat::Float2;
        case VertexFormat::Float3:      return OrbVertexFormat::Float3;
        case VertexFormat::Float4:      return OrbVertexFormat::Float4;
        case VertexFormat::Byte4:       return OrbVertexFormat::Byte4;
        case VertexFormat::Byte4N:      return OrbVertexFormat::Byte4N;
        case VertexFormat::UByte4:      return OrbVertexFormat::UByte4;
        case VertexFormat::UByte4N:     return OrbVertexFormat::UByte4N;
        case VertexFormat::Short2:      return OrbVertexFormat::Short2;
        case VertexFormat::Short2N:     return OrbVertexFormat::Short2N;
        case VertexFormat::Short4:      return OrbVertexFormat::Short4;
        case VertexFormat::Short4N:     return OrbVertexFormat::Short4N;
        default:                        return OrbVertexFormat::Invalid;
    }
}

//------------------------------------------------------------------------------
static OrbAnimKeyFormat::Enum
toOrbAnimKeyFormat(IRep::KeyType::Enum t) {
    switch (t) {
        case IRep::KeyType::Float:      return OrbAnimKeyFormat::Float;
        case IRep::KeyType::Float2:     return OrbAnimKeyFormat::Float2;
        case IRep::KeyType::Float3:     return OrbAnimKeyFormat::Float3;
        case IRep::KeyType::Float4:     return OrbAnimKeyFormat::Float4;
        case IRep::KeyType::Quaternion: return OrbAnimKeyFormat::Quaternion;
        default:                        return OrbAnimKeyFormat::Invalid;
    }
}

//------------------------------------------------------------------------------
static uint32_t
roundup4(uint32_t val) {
    return (val + 3) & ~3;
}

//------------------------------------------------------------------------------
void
OrbSaver::Save(const std::string& path, const IRep& irep) {

    // setup the destination layout, this is the cross-section of
    // the requested layout, and what's actually in the IRep
    this->DstLayout.Components.clear();
    for (const auto& comp : this->Layout.Components) {
        if (irep.HasVertexAttr(comp.Attr)) {
            this->DstLayout.Components.push_back(comp);
        }
    }

    FILE* fp = fopen(path.c_str(), "wb");
    Log::FailIf(!fp, "Failed to open file '%s'\n", path.c_str());

    uint32_t offset = sizeof(OrbHeader);

    // setup the header with offset and numers of items
    OrbHeader hdr;
    hdr.Magic = 'ORB1';
    hdr.VertexComponentOffset = offset;
    hdr.NumVertexComponents = this->DstLayout.Components.size();
    offset += sizeof(OrbVertexComponent) * hdr.NumVertexComponents;
    hdr.ValuePropOffset = offset;
    hdr.NumValueProps = irep.NumValueProps();
    offset += sizeof(OrbValueProperty) * hdr.NumValueProps;
    hdr.TexturePropOffset = offset;
    hdr.NumTextureProps = irep.NumTextureProps();
    offset += sizeof(OrbTextureProperty) * hdr.NumTextureProps;
    hdr.MaterialOffset = offset;
    hdr.NumMaterials = irep.Materials.size();
    offset += sizeof(OrbMaterial) * hdr.NumMaterials;
    hdr.MeshOffset = offset;
    hdr.NumMeshes = irep.NumMeshes();
    offset += sizeof(OrbMesh) * hdr.NumMeshes;
    hdr.BoneOffset = offset;
    hdr.NumBones = irep.Bones.size();
    offset += sizeof(OrbBone) * hdr.NumBones;
    hdr.NodeOffset = offset;
    hdr.NumNodes = irep.Nodes.size();
    offset += sizeof(OrbNode) * hdr.NumNodes;
    hdr.AnimKeyComponentOffset = offset;
    hdr.NumAnimKeyComponents = irep.NumAnimCurvesPerClip();
    offset += sizeof(OrbAnimKeyComponent) * hdr.NumAnimKeyComponents;
    hdr.AnimCurveOffset = offset;
    hdr.NumAnimCurves = irep.NumAnimCurves();
    offset += sizeof(OrbAnimCurve) * hdr.NumAnimCurves;
    hdr.AnimClipOffset = offset;
    hdr.NumAnimClips = irep.AnimClips.size();
    offset += sizeof(OrbAnimClip) * hdr.NumAnimClips;
    hdr.VertexDataOffset = offset;
    hdr.VertexDataSize = irep.NumVertices() * this->DstLayout.ByteSize();
    offset += hdr.VertexDataSize;
    hdr.IndexDataOffset = offset;
    hdr.IndexDataSize = roundup4(irep.NumIndices() * sizeof(uint16_t));
    offset += hdr.IndexDataSize;
    hdr.AnimKeyDataOffset = offset;
    hdr.AnimKeyDataSize = roundup4(irep.AnimKeyDataSize() / 2);   // anim keys are 16-bit signed normalized
    offset += hdr.AnimKeyDataSize;
    hdr.StringPoolDataOffset = offset;
    hdr.StringPoolDataSize = 0;     // this will be filled in at the end!
    for (int i = 0; i < 3; i++) {
        hdr.VertexMagnitude[i] = irep.VertexMagnitude[i];
    }
    fwrite(&hdr, 1, sizeof(hdr), fp);

    // write vertex components
    Log::FailIf(ftell(fp) != hdr.VertexComponentOffset, "File offset error (VertexComponentOffset)\n");
    for (const auto& src : this->DstLayout.Components) {
        OrbVertexComponent dst;
        dst.Attr = toOrbVertexAttr(src.Attr);
        dst.Format = toOrbVertexFormat(src.Format);
        fwrite(&dst, 1, sizeof(dst), fp);
    }

    // write value properties
    {
        Log::FailIf(ftell(fp) != hdr.ValuePropOffset, "File offset error (ValuePropOffset)\n");
        for (const auto& mat : irep.Materials) {
            for (const auto& src : mat.Values) {
                OrbValueProperty dst;
                dst.Name = addString(src.Name);
                dst.Dim = IRep::PropType::NumFloats(src.Type);
                for (int i = 0; i < 4; i++) {
                    dst.Value[i] = src.Value[i];
                }
                fwrite(&dst, 1, sizeof(dst), fp);
            }
        }
    }

    // write texture properties
    Log::FailIf(ftell(fp) != hdr.TexturePropOffset, "File offset error (TexturePropOffset)\n");
    for (const auto& mat : irep.Materials) {
        for (const auto& src : mat.Textures) {
            OrbTextureProperty dst;
            dst.Name = addString(src.Name);
            dst.Location = addString(src.Location);
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    }

    // write materials
    {
        Log::FailIf(ftell(fp) != hdr.MaterialOffset, "File offset error (MaterialOffset)\n");
        uint32_t valPropIndex = 0;
        uint32_t texPropIndex = 0;
        for (const auto& src : irep.Materials) {
            OrbMaterial dst;
            dst.Name = addString(src.Name);
            dst.Shader = addString(src.Shader);
            dst.FirstValueProp = valPropIndex;
            dst.NumValueProps = src.Values.size();
            dst.FirstTextureProp = texPropIndex;
            dst.NumTextureProps = src.Textures.size();
            valPropIndex += dst.NumValueProps;
            texPropIndex += dst.NumTextureProps;
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    }

    // write meshes
    int curVertex = 0;
    int curIndex = 0;
    const int srcVertexStride = irep.VertexStrideBytes() / sizeof(float);
    Log::FailIf(ftell(fp) != hdr.MeshOffset, "File offset error (MeshOffset)\n");
    for (const auto& node : irep.Nodes) {
        for (const auto& src : node.Meshes) {
            OrbMesh dst;
            dst.Material = src.Material;
            dst.FirstVertex = curVertex;
            dst.NumVertices = src.VertexData.size() / srcVertexStride;
            dst.FirstIndex = curIndex;
            dst.NumIndices = src.IndexData.size();
            fwrite(&dst, 1, sizeof(dst), fp);
            curVertex += dst.NumVertices;
            curIndex += dst.NumIndices;
        }
    }

    // write bones
    Log::FailIf(ftell(fp) != hdr.BoneOffset, "File offset error (BoneOffset)\n");
    for (const auto& src : irep.Bones) {
        OrbBone dst;
        dst.Name = addString(src.Name);
        dst.Parent = src.Parent;
        for (int i = 0; i < 3; i++) {
            dst.Translate[i] = src.Translate[i];
            dst.Scale[i] = src.Scale[i];
        }
        for (int i = 0; i < 4; i++) {
            dst.Rotate[i] = src.Rotate[i];
        }
        fwrite(&dst, 1, sizeof(dst), fp);
    }

    // write nodes
    {
        Log::FailIf(ftell(fp) != hdr.NodeOffset, "File offset error (NodeOffset)\n");
        uint32_t meshIndex = 0;
        for (const auto& src : irep.Nodes) {
            OrbNode dst;
            dst.Name = addString(src.Name);
            dst.Parent = src.Parent;
            dst.FirstMesh = meshIndex;
            dst.NumMeshes = src.Meshes.size();
            for (int i = 0; i < 3; i++) {
                dst.Translate[i] = src.Translate[i];
                dst.Scale[i] = src.Scale[i];
            }
            for (int i = 0; i < 4; i++) {
                dst.Rotate[i] = src.Rotate[i];
            }
            fwrite(&dst, 1, sizeof(dst), fp);
            meshIndex += dst.NumMeshes;
        }
    }

    // write anim key formats
    Log::FailIf(ftell(fp) != hdr.AnimKeyComponentOffset, "File offset error (AnimKeyComponentOffset)\n");
    if (!irep.AnimClips.empty()) {
        for (const auto& curve : irep.AnimClips[0].Curves) {
            OrbAnimKeyComponent dst;
            dst.KeyFormat = toOrbAnimKeyFormat(curve.Type);
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    }

    // write anim curves
    Log::FailIf(ftell(fp) != hdr.AnimCurveOffset, "File offset error (AnimCurveOffset)\n");
    for (int clipIndex = 0; clipIndex < int(irep.AnimClips.size()); clipIndex++) {
        const auto& clip = irep.AnimClips[clipIndex];
        for (int curveIndex = 0; curveIndex < int(clip.Curves.size()); curveIndex++) {
            const auto& curve = clip.Curves[curveIndex];
            OrbAnimCurve dst;
            if (curve.IsStatic) {
                dst.KeyOffset = -1;
            }
            else {
                dst.KeyOffset = irep.AnimKeyOffset(clipIndex, curveIndex) / 2; // anim keys are 16-bit signed normalized
                Log::FailIf(dst.KeyOffset >= int(hdr.AnimKeyDataSize), "Anim key offset too big\n");
            }
            for (int i = 0; i < 4; i++) {
                dst.StaticKey[i] = curve.StaticKey[i];
                dst.Magnitude[i] = curve.Magnitude[i];
            }
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    } 

    // write anim clips
    {
        uint32_t curveIndex = 0;
        Log::FailIf(ftell(fp) != hdr.AnimClipOffset, "File offset error (AnimClipOffset)\n");
        for (int clipIndex = 0; clipIndex < int(irep.AnimClips.size()); clipIndex++) {
            const auto& src = irep.AnimClips[clipIndex];
            OrbAnimClip dst;
            dst.Name = addString(src.Name);
            dst.KeyDuration = src.KeyDuration;
            dst.Length = irep.AnimClipLength(clipIndex);
            dst.FirstCurve = curveIndex;
            dst.NumCurves = src.Curves.size();
            curveIndex += dst.NumCurves;
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    }

    // write the vertex data
    {
        Log::FailIf(ftell(fp) != hdr.VertexDataOffset, "File offset error (VertexDataOffset)\n");
        uint8_t encodeSpace[1024];
        Log::FailIf(this->DstLayout.ByteSize() >= int(sizeof(encodeSpace)), "Dst vertex stride too big\n");
        int allEncodedBytes = 0;
        const glm::vec4 scaleOne(1.0f);
        const glm::vec4 scalePos(1.0f/irep.VertexMagnitude, 1.0f);
        for (const auto& node : irep.Nodes) {
            for (const auto& mesh : node.Meshes) {
                const float* srcStart = &mesh.VertexData[0];
                const int numVertices = mesh.VertexData.size() / srcVertexStride;
                for (int i = 0; i < numVertices; i++) {
                    uint8_t* dstPtr = encodeSpace;
                    for (const auto& srcComp : irep.VertexComponents) {
                        if (!this->DstLayout.HasAttr(srcComp.Attr)) {
                            continue;
                        }
                        VertexFormat::Code dstFmt = this->DstLayout.AttrFormat(srcComp.Attr);
                        const float* srcPtr = srcStart + i*srcVertexStride + srcComp.Offset/4;
                        const int numSrcItems = VertexFormat::NumItems(srcComp.Format);
                        switch (dstFmt) {
                            case VertexFormat::Float:
                                dstPtr = VertexCodec::Encode<VertexFormat::Float>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Float2:
                                dstPtr = VertexCodec::Encode<VertexFormat::Float2>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Float3:
                                dstPtr = VertexCodec::Encode<VertexFormat::Float3>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Float4:
                                dstPtr = VertexCodec::Encode<VertexFormat::Float4>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Byte4:
                                dstPtr = VertexCodec::Encode<VertexFormat::Byte4>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Byte4N:
                                dstPtr = VertexCodec::Encode<VertexFormat::Byte4N>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::UByte4:
                                dstPtr = VertexCodec::Encode<VertexFormat::UByte4>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::UByte4N:
                                dstPtr = VertexCodec::Encode<VertexFormat::UByte4N>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Short2:
                                dstPtr = VertexCodec::Encode<VertexFormat::Short2>(dstPtr, scalePos, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Short2N:
                                // FIXME: currently hardcoded for 3.15 fixed-point UV coords!
                                dstPtr = VertexCodec::Encode<VertexFormat::Short2N>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Short4:
                                dstPtr = VertexCodec::Encode<VertexFormat::Short4>(dstPtr, scaleOne, srcPtr, numSrcItems);
                                break;
                            case VertexFormat::Short4N:
                                // FIXME: currently hardcoded for vertex positions
                                dstPtr = VertexCodec::Encode<VertexFormat::Short4N>(dstPtr, scalePos, srcPtr, numSrcItems);
                                break;
                            default: break;
                        }
                    }
                    const int numEncodedBytes = dstPtr - encodeSpace;
                    allEncodedBytes += numEncodedBytes;
                    fwrite(encodeSpace, 1, numEncodedBytes, fp);
                }
            }
        }
        Log::FailIf(allEncodedBytes != int(hdr.VertexDataSize), "Encoded destination length error!\n");
    }

    // write vertex indices
    {
        Log::FailIf(ftell(fp) != hdr.IndexDataOffset, "File offset error (IndexDataOffset)\n");
        uint16_t baseVertexIndex = 0;
        int numBytes = 0;
        for (const auto& node : irep.Nodes) {
            for (const auto& mesh : node.Meshes) {
                for (uint16_t li : mesh.IndexData) {
                    uint16_t vi = li + baseVertexIndex;
                    fwrite(&vi, 1, sizeof(vi), fp);
                    numBytes += 2;
                }
                baseVertexIndex += mesh.VertexData.size() / srcVertexStride;
            }
        }
        if ((numBytes & 3) != 0) {
            uint16_t padding = 0;
            fwrite(&padding, 1, sizeof(padding), fp);
        }
    }

    // write animation keys
    Log::FailIf(ftell(fp) != hdr.AnimKeyDataOffset, "File offset error (AnimKeyDataSize)\n");
    for (int clipIndex = 0; clipIndex < int(irep.AnimClips.size()); clipIndex++) {
        const auto& clip = irep.AnimClips[clipIndex];
        const int clipLength = irep.AnimClipLength(clipIndex);
        for (int keyIndex = 0; keyIndex < clipLength; keyIndex++) {
            for (const auto& curve : clip.Curves) {
                if (!curve.IsStatic) {
                    const int num = IRep::KeyType::NumComponents(curve.Type);
                    for (int i = 0; i < num; i++) {
                        float f = 0.0f;
                        if (curve.Magnitude[i] > 0.0f) {
                            f = curve.Keys[keyIndex][i] / curve.Magnitude[i];
                        }
                        // f is now between -1.0 and +1.0
                        glm::i16 p = glm::round(glm::clamp(f*32767.0f, -32768.0f, 32767.0f));
                        fwrite(&p, 1, sizeof(p), fp);
                    }
                }
            }
        }
    }
    // 2-bytes padding if animkey data size isn't multiple of 4
    if (((irep.AnimKeyDataSize() / 2) & 3) != 0) {
        int16_t padding = 0;
        fwrite(&padding, 1, sizeof(padding), fp);
    }

    // write string pool
    {
        Log::FailIf(ftell(fp) != hdr.StringPoolDataOffset, "File offset error (StringPoolDataOffset)\n");
        uint32_t stringPoolDataSize = 0;
        for (const auto& str : this->strings) {
            uint32_t strDataLength = str.length()+1;
            stringPoolDataSize += strDataLength;
            fwrite(str.c_str(), 1, str.length()+1, fp);
        }
        // patch the written string pool size back into the hdr
        Log::FailIf(stringPoolDataSize != (ftell(fp)-hdr.StringPoolDataOffset), "String pool size mismatch!\n");
        fseek(fp, offsetof(OrbHeader, StringPoolDataSize), SEEK_SET);
        fwrite(&stringPoolDataSize, 1, sizeof(stringPoolDataSize), fp);
        fseek(fp, 0, SEEK_END);
    }
    fclose(fp);
}
