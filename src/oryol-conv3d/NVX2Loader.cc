//------------------------------------------------------------------------------
//  NVX2Loader.cc
//------------------------------------------------------------------------------
#include "NVX2Loader.h"
#include "ExportUtil/Log.h"
#include "LoadUtil.h"
#include "ExportUtil/Vertex.h"
#include "ExportUtil/VertexCodec.h"

using namespace OryolTools;

// vertex attr, source format, dest format
static struct NVX2Loader::Component VertexComponentMapping[] = {
    { VertexAttr::Position, VertexFormat::Float3, VertexFormat::Float3 },                       // N2Coord
    { VertexAttr::Normal, VertexFormat::Float3, VertexFormat::Float3 },                         // N2Normal
    { VertexAttr::Normal, VertexFormat::UByte4, VertexFormat::Float3, 1.0f/127.0f, -1.0f },     // N2NormalUB4N
    { VertexAttr::TexCoord0, VertexFormat::Float2, VertexFormat::Float2 },                      // N2Uv0
    { VertexAttr::TexCoord0, VertexFormat::Short2, VertexFormat::Float2, 1.0f/8192.0f, 0.0f },  // N2Uv0S2
    { VertexAttr::TexCoord1, VertexFormat::Float2, VertexFormat::Float2 },                      // N2Uv2
    { VertexAttr::TexCoord1, VertexFormat::Short2, VertexFormat::Float2, 1.0f/8192.0f, 0.0f },  // N2Uv1S2
    { VertexAttr::TexCoord2, VertexFormat::Float2, VertexFormat::Float2 },                      // N2Uv2
    { VertexAttr::TexCoord2, VertexFormat::Short2, VertexFormat::Float2, 1.0f/8192.0f, 0.0f },  // N2Uv2S2
    { VertexAttr::TexCoord3, VertexFormat::Float2, VertexFormat::Float2 },                      // N2Uv3
    { VertexAttr::TexCoord3, VertexFormat::Short2, VertexFormat::Float2, 1.0f/8192.0f, 0.0f },  // N2Uv3S2
    { VertexAttr::Color0, VertexFormat::Float4, VertexFormat::Float4 },                         // N2Color
    { VertexAttr::Color0, VertexFormat::UByte4N, VertexFormat::Float4 },                        // N2ColorUB4N
    { VertexAttr::Tangent, VertexFormat::Float3, VertexFormat::Float3 },                        // N2Tangent
    { VertexAttr::Tangent, VertexFormat::UByte4, VertexFormat::Float3, 1.0f/127.0f, -1.0f },    // N2TangentUB4N
    { VertexAttr::Binormal, VertexFormat::Float3, VertexFormat::Float3 },                       // N2Binormal
    { VertexAttr::Binormal, VertexFormat::UByte4, VertexFormat::Float3, 1.0f/127.0f, -1.0f },   // N2BinormalUB4N
    { VertexAttr::Weights, VertexFormat::Float4, VertexFormat::Float4 },                        // N2Weights
    { VertexAttr::Weights, VertexFormat::UByte4N, VertexFormat::Float4 },                       // N2WeightUB4N
    { VertexAttr::Indices, VertexFormat::Float4, VertexFormat::Float4 },                        // N2JIndices
    { VertexAttr::Indices, VertexFormat::UByte4, VertexFormat::Float4 },                        // N2JIndicesUB4
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
int
NVX2Loader::NumVertices() const {
    int numVertices = 0;
    for (const auto& mesh : this->Meshes) {
        numVertices += mesh.NumVertices;
    }
    return numVertices;
}

//------------------------------------------------------------------------------
int
NVX2Loader::NumIndices() const {
    int numIndices = 0;
    for (const auto& mesh : this->Meshes) {
        numIndices += mesh.NumIndices;
    }
    return numIndices;
}

//------------------------------------------------------------------------------
int
NVX2Loader::VertexStride() const {
    if (!this->Meshes.empty()) {
        return this->Meshes[0].DstStride;
    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
void
NVX2Loader::ValidateVertexLayouts() const {
    if (this->Meshes.empty()) {
        return;
    }
    const std::vector<Component>& comps = this->Meshes[0].Components;
    const int dstStride = this->Meshes[0].DstStride;
    for (const auto& mesh : this->Meshes) {
        Log::FailIf(comps.size() != mesh.Components.size(), "NVX2Loader: meshes have different vertex layout!\n");
        for (int i = 0; i < (int)comps.size(); i++) {
            Log::FailIf(comps[i] != mesh.Components[i], "NVX2Loader: meshes have different vertex layout!\n");
        }
        Log::FailIf(mesh.DstStride != dstStride, "NVX2Loader: meshes have different vertex layoyt!\n");
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
    const uint8_t* start = load_file(path);

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
    mesh.VertexData.resize((mesh.NumVertices * mesh.DstStride) / sizeof(float));
    const uint8_t* vxSrcPtr = start+sizeof(Nvx2Header)+nvx2Hdr->NumGroups*sizeof(Nvx2Group);
    float* vxDstPtr = &(mesh.VertexData[0]);
    const float* vxDstEndPtr = &(*mesh.VertexData.end());
    for (int vi = 0; vi < mesh.NumVertices; vi++) {
        for (const auto& comp : mesh.Components) {
            float* dst = &(vxDstPtr[comp.DstOffset/sizeof(float)]);
            const uint8_t* src = &(vxSrcPtr[comp.SrcOffset]);
            const int numDstComps = VertexFormat::NumItems(comp.DstFormat);
            switch (comp.SrcFormat) {
                case VertexFormat::Float2:
                    VertexCodec::Decode<VertexFormat::Float2>(dst, comp.Scale, comp.Bias, src, 2, numDstComps);
                    break;
                case VertexFormat::Float3:
                    VertexCodec::Decode<VertexFormat::Float3>(dst, comp.Scale, comp.Bias, src, 3, numDstComps);
                    break;
                case VertexFormat::Float4:
                    VertexCodec::Decode<VertexFormat::Float4>(dst, comp.Scale, comp.Bias, src, 4, numDstComps);
                    break;
                case VertexFormat::Byte4N:
                    VertexCodec::Decode<VertexFormat::Byte4N>(dst, comp.Scale, comp.Bias, src, 4, numDstComps);
                    break;
                case VertexFormat::Byte4:
                    VertexCodec::Decode<VertexFormat::Byte4>(dst, comp.Scale, comp.Bias, src, 4, numDstComps);
                    break;
                case VertexFormat::UByte4N:
                    VertexCodec::Decode<VertexFormat::UByte4N>(dst, comp.Scale, comp.Bias, src, 4, numDstComps);
                    break;
                case VertexFormat::UByte4:
                    VertexCodec::Decode<VertexFormat::UByte4>(dst, comp.Scale, comp.Bias, src, 4, numDstComps);
                    break;
                case VertexFormat::Short2:
                    VertexCodec::Decode<VertexFormat::Short2>(dst, comp.Scale, comp.Bias, src, 2, numDstComps);
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

    // copy triangle indices over, need to reverse the winding order
    Log::FailIf((mesh.NumIndices % 3) != 0, "Number of indices not a multiple of 3!\n");
    mesh.IndexData.resize(mesh.NumIndices, 0);
    const uint16_t* ixSrcPtr = (const uint16_t*) vxSrcPtr;
    uint16_t* ixDstPtr = &(mesh.IndexData[0]);
    for (int i = 0; i < (mesh.NumIndices/3); i++) {
        ixDstPtr[i*3 + 0] = ixSrcPtr[i*3 + 2];
        ixDstPtr[i*3 + 1] = ixSrcPtr[i*3 + 1];
        ixDstPtr[i*3 + 2] = ixSrcPtr[i*3 + 0];
    }
    free_file_data(start);
}

//------------------------------------------------------------------------------
void
NVX2Loader::GlobalizeJointIndices(const std::string& meshName, int primGroupIndex, const std::vector<int>& jointPalette) {
    for (auto& mesh : this->Meshes) {
        if (mesh.Name != meshName) {
            continue;
        }
        int compOffset = -1;
        for (const auto& comp : mesh.Components) {
            if (comp.Attr == VertexAttr::Indices) {
                compOffset = comp.DstOffset / sizeof(float);
                break;
            }
        }
        if (-1 == compOffset) {
            continue;
        }
        const int firstVertex = mesh.PrimGroups[primGroupIndex].FirstVertex;
        const int numVertices = mesh.PrimGroups[primGroupIndex].NumVertices;
        for (int vtx = firstVertex; vtx < (firstVertex+numVertices); vtx++) {
            int vtxOffset = vtx * (mesh.DstStride/sizeof(float)) + compOffset;
            for (int i = 0; i < 4; i++, vtxOffset++) {
                float floatJointPaletteIndex = mesh.VertexData[vtxOffset];
                int jointPaletteIndex = int(floatJointPaletteIndex + 0.5f);
                int globalJointIndex = jointPalette[jointPaletteIndex];
                mesh.VertexData[vtxOffset] = (float) globalJointIndex;
            }
        }
    }
}
