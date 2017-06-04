//------------------------------------------------------------------------------
//  NVX2Loader.cc
//------------------------------------------------------------------------------
#include "NVX2Loader.h"
#include "ExportUtil/Log.h"
#include "Util.h"
#include "ExportUtil/Vertex.h"
#include "ExportUtil/VertexCodec.h"

using namespace OryolTools;

// vertex attr, source format, dest format
static struct NVX2Loader::Component VertexComponentMapping[] = {
    { VertexAttr::Position, VertexFormat::Float3, VertexFormat::Float3 },   // N2Coord
    { VertexAttr::Normal, VertexFormat::Float3, VertexFormat::Float3 },     // N2Normal
    { VertexAttr::Normal, VertexFormat::Byte4N, VertexFormat::Float3 },     // N2NormalUB4N
    { VertexAttr::TexCoord0, VertexFormat::Float2, VertexFormat::Float2 },  // N2Uv0
    { VertexAttr::TexCoord0, VertexFormat::Short2, VertexFormat::Float2 },  // N2Uv0S2
    { VertexAttr::TexCoord1, VertexFormat::Float2, VertexFormat::Float2 },  // N2Uv2
    { VertexAttr::TexCoord1, VertexFormat::Short2, VertexFormat::Float2 },  // N2Uv1S2
    { VertexAttr::TexCoord2, VertexFormat::Float2, VertexFormat::Float2 },  // N2Uv2
    { VertexAttr::TexCoord2, VertexFormat::Short2, VertexFormat::Float2 },  // N2Uv2S2
    { VertexAttr::TexCoord3, VertexFormat::Float2, VertexFormat::Float2 },  // N2Uv3
    { VertexAttr::TexCoord3, VertexFormat::Short2, VertexFormat::Float2 },  // N2Uv3S2
    { VertexAttr::Color0, VertexFormat::Float4, VertexFormat::Float4 },     // N2Color
    { VertexAttr::Color0, VertexFormat::UByte4N, VertexFormat::Float4 },    // N2ColorUB4N
    { VertexAttr::Tangent, VertexFormat::Float3, VertexFormat::Float3 },    // N2Tangent
    { VertexAttr::Tangent, VertexFormat::Byte4N, VertexFormat::Float3 },    // N2TangentUB4N
    { VertexAttr::Binormal, VertexFormat::Float3, VertexFormat::Float3 },   // N2Binormal
    { VertexAttr::Binormal, VertexFormat::Byte4N, VertexFormat::Float3 },   // N2BinormalUB4N
    { VertexAttr::Weights, VertexFormat::Float4, VertexFormat::Float4 },    // N2Weights
    { VertexAttr::Weights, VertexFormat::UByte4N, VertexFormat::Float4 },   // N2WeightUB4N
    { VertexAttr::Indices, VertexFormat::Float4, VertexFormat::Float4 },    // N2JIndices
    { VertexAttr::Indices, VertexFormat::UByte4, VertexFormat::Float4 },    // N2JIndicesUB4
};

//------------------------------------------------------------------------------
bool
NVX2Loader::HasMesh(const std::string& nvx2AssetName) const {
    for (const auto& mesh : this->Meshes) {
        if (mesh.Name == nvx2AssetName) {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
const NVX2Loader::Mesh&
NVX2Loader::MeshByName(const std::string& nvx2AssetName) const {
    Log::FailIf(!this->HasMesh(nvx2AssetName), "Mesh '%s' not found!\n", nvx2AssetName.c_str());
    for (const auto& mesh : this->Meshes) {
        if (mesh.Name == nvx2AssetName) {
            return mesh;
        }
    }
    static NVX2Loader::Mesh dummy;
    return dummy;
}

//------------------------------------------------------------------------------
NVX2Loader::PrimGroup
NVX2Loader::AbsPrimGroup(const std::string& nvx2AssetName, int localPrimGroupIndex) const {
    int baseVertex = 0;
    int baseIndex = 0;
    PrimGroup primGroup;
    for (const auto& mesh : this->Meshes) {
        if (mesh.Name == nvx2AssetName) {
            primGroup = mesh.PrimGroups[localPrimGroupIndex];
            primGroup.FirstVertex += baseVertex;
            primGroup.FirstIndex += baseIndex;
            break;
        }
        else {
            baseVertex += mesh.NumVertices;
            baseIndex += mesh.NumIndices;
        }
    }
    return primGroup;
}


//------------------------------------------------------------------------------
void
NVX2Loader::ValidateVertexLayouts() const {
    if (this->Meshes.empty()) {
        return;
    }
    const std::vector<Component>& comps = this->Meshes[0].Components;
    for (const auto& mesh : this->Meshes) {
        Log::FailIf(comps.size() != mesh.Components.size(), "NVX2Loader: meshes have different vertex layout!\n");
        for (int i = 0; i < (int)comps.size(); i++) {
            Log::FailIf(comps[i] != mesh.Components[i], "NVX2Loader: meshes have different vertex layout!\n");
        }
    }
}

//------------------------------------------------------------------------------
void
NVX2Loader::Clear() {
    this->Layout.Components.clear();
    this->Meshes.clear();
}

//------------------------------------------------------------------------------
void
NVX2Loader::Load(const std::string& nvx2AssetName, const std::string& n3AssetDir) {
    Log::FailIf(this->Layout.Components.empty(), "NVX2Loader: please set a vertex layout!\n");

    // mesh might already be loaded
    if (this->HasMesh(nvx2AssetName)) {
        return;
    }

    // load the entire file into memory
    std::string path = n3AssetDir + "/meshes/" + nvx2AssetName;
    FILE* fp = fopen(path.c_str(), "rb");
    Log::FailIf(!fp, "Failed to open file '%s'\n", path.c_str());
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    const uint8_t* start = (const uint8_t*) malloc(size);
    int bytesRead = fread((void*)start, 1, size, fp);
    Log::FailIf(bytesRead != size, "Failed reading file '%s' into memory\n", path.c_str());
    fclose(fp);

    // parse the header
    const Nvx2Header* nvx2Hdr = (const Nvx2Header*) start;
    Log::FailIf(nvx2Hdr->Magic != 'NVX2', "NVX2 magic number mismatch in '%s'\n", path.c_str());
    this->Meshes.push_back(Mesh());
    auto& mesh = this->Meshes.back();
    mesh.Name = nvx2AssetName;
    mesh.NumVertices = nvx2Hdr->NumVertices;
    mesh.NumIndices = nvx2Hdr->NumTriangles * 3;
    for (int i = 0; i < N2NumVertexComponents; i++) {
        const uint32_t n2Comp = (1<<i);
        if (nvx2Hdr->VertexComponentMask & n2Comp) {
            NVX2Loader::Component comp = VertexComponentMapping[i];
            if (this->Layout.HasAttr(comp.Attr)) {
                comp.SrcOffset = mesh.SrcStride;
                comp.DstOffset = mesh.DstStride;
                mesh.Components.push_back(comp);
                mesh.DstStride += VertexFormat::ByteSize(comp.DstFormat);
            }
            mesh.SrcStride += VertexFormat::ByteSize(comp.SrcFormat);
        }
    }
    mesh.PrimGroups.reserve(nvx2Hdr->NumGroups);
    for (int i = 0; i < (int)nvx2Hdr->NumGroups; i++) {
        const Nvx2Group* nvx2Grp = (const Nvx2Group*) ((start+sizeof(Nvx2Header)) + i*sizeof(Nvx2Group));
        PrimGroup pg;
        pg.FirstVertex = nvx2Grp->FirstVertex;
        pg.NumVertices = nvx2Grp->NumVertices;
        pg.FirstIndex = nvx2Grp->FirstTriangle * 3;
        pg.NumIndices = nvx2Grp->NumTriangles * 3;
        mesh.PrimGroups.push_back(pg);
    }

    // decode vertices
    mesh.VertexData.resize(mesh.NumVertices * mesh.DstStride);
    const uint8_t* vxSrcPtr = start+sizeof(Nvx2Header)+nvx2Hdr->NumGroups*sizeof(Nvx2Group);
    float* vxDstPtr = &(mesh.VertexData[0]);
    const float* vxDstEndPtr = &(*mesh.VertexData.end());
    for (int vi = 0; vi < mesh.NumVertices; vi++) {
        for (const auto& comp : mesh.Components) {
            float* dst = &(vxDstPtr[comp.DstOffset/sizeof(float)]);
            const uint8_t* src = &(vxSrcPtr[comp.SrcOffset]);
            switch (comp.SrcFormat) {
                case VertexFormat::Float2:
                    VertexCodec::Decode<VertexFormat::Float2>(dst, 1.0f, src, 2);
                    break;
                case VertexFormat::Float3:
                    VertexCodec::Decode<VertexFormat::Float3>(dst, 1.0f, src, 3);
                    break;
                case VertexFormat::Float4:
                    VertexCodec::Decode<VertexFormat::Float4>(dst, 1.0f, src, 4);
                    break;
                case VertexFormat::Byte4N:
                    VertexCodec::Decode<VertexFormat::Byte4N>(dst, 1.0f, src, 4);
                    break;
                case VertexFormat::Byte4:
                    VertexCodec::Decode<VertexFormat::Byte4>(dst, 1.0f, src, 4);
                    break;
                case VertexFormat::UByte4N:
                    VertexCodec::Decode<VertexFormat::UByte4N>(dst, 1.0f, src, 4);
                    break;
                case VertexFormat::UByte4:
                    VertexCodec::Decode<VertexFormat::UByte4>(dst, 1.0f, src, 4);
                    break;
                case VertexFormat::Short2:
                    VertexCodec::Decode<VertexFormat::Short2>(dst, 1.0f/4096.0f, src, 2);
                    break;
                default:
                    break;
            }
        }
        vxSrcPtr += mesh.SrcStride;
        vxDstPtr += mesh.DstStride / sizeof(float);
        if (vxDstPtr > vxDstEndPtr) {
            Log::Fatal("Vertex data size mismatch\n");
        }
    }

    // copy index data over
    mesh.IndexData.resize(mesh.NumIndices, 0);
    const uint16_t* ixSrcPtr = (const uint16_t*) vxSrcPtr;
    uint16_t* ixDstPtr = &(mesh.IndexData[0]);
    for (int i = 0; i < mesh.NumIndices; i++) {
        ixDstPtr[i] = ixSrcPtr[i];
    }

    free((void*)start);
}
