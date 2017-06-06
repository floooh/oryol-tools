//------------------------------------------------------------------------------
//  OrbSaver.cc
//------------------------------------------------------------------------------
#include "OrbSaver.h"
#include "ExportUtil/Log.h"
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
VertexFormat::Code
OrbSaver::mapVertexFormat(VertexAttr::Code attr, VertexFormat::Code fmt) const {
    // check if a vertex format mapping has been defined for a vertex attribute,
    // if yes, return mapped format, otherwise return original format
    if (VertexFormats.find(attr) != VertexFormats.end()) {
        return VertexFormats.at(attr);
    }
    else {
        return fmt;
    }
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
int
OrbSaver::vertexStrideInBytes(const IRep& irep) const {
    // compute the size of the destination vertex layout in num bytes
    int size = 0;
    for (const auto& comp : irep.VertexComponents) {
        size += VertexFormat::ByteSize(mapVertexFormat(comp.Attr, comp.Format));
    }
    return size;
}

//------------------------------------------------------------------------------
static uint32_t
roundup4(uint32_t val) {
    return (val + 3) & ~3;
}

//------------------------------------------------------------------------------
void
OrbSaver::Save(const std::string& path, const IRep& irep) {
    FILE* fp = fopen(path.c_str(), "wb");
    Log::FailIf(!fp, "Failed to open file '%s'\n", path.c_str());

    uint32_t offset = sizeof(OrbHeader);

    // setup the header with offset and numers of items
    OrbHeader hdr;
    hdr.Magic = 'ORB1';
    hdr.VertexComponentOffset = offset;
    hdr.NumVertexComponents = irep.VertexComponents.size();
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
    hdr.NumMeshes = irep.Meshes.size();
    offset += sizeof(OrbMesh) * hdr.NumMeshes;
    hdr.BoneOffset = offset;
    hdr.NumBones = irep.Bones.size();
    offset += sizeof(OrbBone) * hdr.NumBones;
    hdr.AnimKeyComponentOffset = offset;
    hdr.NumAnimKeyComponents = irep.KeyComponents.size();
    offset += sizeof(OrbAnimKeyFormat) * hdr.NumAnimKeyComponents;
    hdr.AnimCurveOffset = offset;
    hdr.NumAnimCurves = irep.NumAnimCurves();
    offset += sizeof(OrbAnimCurve) * hdr.NumAnimCurves;
    hdr.AnimClipOffset = offset;
    hdr.NumAnimClips = irep.AnimClips.size();
    offset += sizeof(OrbAnimClip) * hdr.NumAnimClips;
    hdr.ValuePoolOffset = offset;
    hdr.ValuePoolSize = irep.NumPropValues() * sizeof(float);
    offset += hdr.ValuePoolSize;
    const int vertexStrideInFloats = irep.VertexStrideBytes() / sizeof(float);
    Log::FailIf((irep.VertexData.size() % vertexStrideInFloats) != 0, "Vertex data size mismatch!\n");
    const int numVertices = irep.VertexData.size() / vertexStrideInFloats;
    Log::FailIf((irep.NumMeshVertices() != numVertices), "Number of vertices mismatch!\n");
    hdr.VertexDataOffset = offset;
    hdr.VertexDataSize = numVertices * vertexStrideInBytes(irep);
    offset += hdr.VertexDataSize;
    hdr.IndexDataOffset = offset;
    hdr.IndexDataSize = roundup4(irep.IndexData.size() * sizeof(uint16_t));
    offset += hdr.IndexDataSize;
    hdr.AnimKeyDataOffset = offset;
    hdr.AnimKeyDataSize = irep.KeyData.size() * sizeof(float);
    offset += hdr.AnimKeyDataSize;
    hdr.StringPoolDataOffset = offset;
    hdr.StringPoolDataSize = 0;     // this will be filled in at the end!
    fwrite(&hdr, 1, sizeof(hdr), fp);

    // write vertex components
    Log::FailIf(ftell(fp) != hdr.VertexComponentOffset, "File write error (VertexComponentOffset)\n");
    for (const auto& src : irep.VertexComponents) {
        OrbVertexComponent dst;
        dst.Attr = toOrbVertexAttr(src.Attr);
        dst.Format = toOrbVertexFormat(mapVertexFormat(src.Attr, src.Format));
        fwrite(&dst, 1, sizeof(dst), fp);
    }

    // write value properties
    Log::FailIf(ftell(fp) != hdr.ValuePropOffset, "File write error (ValuePropOffset)\n");
    uint32_t valueIndex = 0;
    for (const auto& mat : irep.Materials) {
        for (const auto& src : mat.Values) {
            OrbValueProperty dst;
            dst.Name = addString(src.Name);
            dst.FirstValue = valueIndex;
            dst.NumValues = IRep::PropType::NumFloats(src.Type);
            valueIndex += dst.NumValues;
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    }

    // write texture properties
    Log::FailIf(ftell(fp) != hdr.TexturePropOffset, "File write error (TexturePropOffset)\n");
    for (const auto& mat : irep.Materials) {
        for (const auto& src : mat.Textures) {
            OrbTextureProperty dst;
            dst.Name = addString(src.Name);
            dst.Location = addString(src.Location);
            fwrite(&dst, 1, sizeof(dst), fp);
        }
    }

    // write materials
    Log::FailIf(ftell(fp) != hdr.MaterialOffset, "File write error (MaterialOffset)\n");

    fclose(fp);
}
