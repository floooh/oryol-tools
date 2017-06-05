//------------------------------------------------------------------------------
//  JSONMeshDumper.cc
//------------------------------------------------------------------------------
#include "JSONMeshDumper.h"
#include "ExportUtil/Log.h"
#include "cJSON.h"
#include <cassert>
#include <cstdlib>

using namespace std;

namespace OryolTools {

//------------------------------------------------------------------------------
std::string
JSONMeshDumper::Dump(const Mesh& mesh) {
    assert(mesh.VertexBuffer.IsValid());
    assert(mesh.IndexBuffer.IsValid());
    assert(!mesh.PrimGroups.empty());

    cJSON* jsonRoot = cJSON_CreateObject();
    dumpHeader(jsonRoot, mesh);
    dumpVertices(jsonRoot, mesh);
    dumpIndices(jsonRoot, mesh);

    char* rawStr = cJSON_Print(jsonRoot);
    std::string jsonStr(rawStr);
    std::free(rawStr);
    cJSON_Delete(jsonRoot);
    return jsonStr;    
}

//------------------------------------------------------------------------------
void
JSONMeshDumper::dumpHeader(cJSON* jsonNode, const Mesh& mesh) {

    cJSON_AddItemToObject(jsonNode, "num_vertices", cJSON_CreateNumber(mesh.VertexBuffer.GetNumVertices()));
    cJSON_AddItemToObject(jsonNode, "num_indices", cJSON_CreateNumber(mesh.IndexBuffer.GetNumIndices()));
    cJSON_AddItemToObject(jsonNode, "index_size", cJSON_CreateNumber(mesh.IndexBuffer.GetIndexSize()));

    cJSON* jsonLayout = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "vertex_layout", jsonLayout);
    for (int i = 0; i < VertexAttr::Num; i++) {
        const auto& comp = mesh.VertexBuffer.GetVertexLayout().Components[i];
        if (comp.Format != VertexFormat::Invalid) {
            VertexAttr::Code attr = (VertexAttr::Code) i;
            cJSON* jsonComp = cJSON_CreateObject();
            cJSON_AddItemToArray(jsonLayout, jsonComp);
            cJSON_AddItemToObject(jsonComp, "attr", cJSON_CreateString(VertexAttr::ToString(attr)));
            cJSON_AddItemToObject(jsonComp, "format", cJSON_CreateString(VertexFormat::ToString(comp.Format)));
        }
    }

    cJSON* jsonPrimGroups = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "prim_groups", jsonPrimGroups);
    for (const auto& group : mesh.PrimGroups) {
        cJSON* jsonPrimGroup = cJSON_CreateObject();
        cJSON_AddItemToArray(jsonPrimGroups, jsonPrimGroup);
        cJSON_AddItemToObject(jsonPrimGroup, "type", cJSON_CreateString(PrimitiveGroup::TypeToString(group.Type).c_str()));
        cJSON_AddItemToObject(jsonPrimGroup, "base_element", cJSON_CreateNumber(group.BaseElement));
        cJSON_AddItemToObject(jsonPrimGroup, "num_elements", cJSON_CreateNumber(group.NumElements));
    }
}

//------------------------------------------------------------------------------
void
JSONMeshDumper::dumpVertices(cJSON* jsonNode, const Mesh& mesh) {

    // NOTE: this will be terribly slow for big meshes, but
    // JSON dumping is only meant for debugging

    cJSON* jsonVertices = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "vertices", jsonVertices);
    const VertexLayout& layout = mesh.VertexBuffer.GetVertexLayout();
    const uint8_t* ptr = mesh.VertexBuffer.GetDataPointer();
    const int numVerts = mesh.VertexBuffer.GetNumVertices();
    for (int vertIndex = 0; vertIndex < numVerts; vertIndex++) {
        cJSON* jsonVertex = cJSON_CreateArray();
        cJSON_AddItemToArray(jsonVertices, jsonVertex);
        for (int attr = 0; attr < VertexAttr::Num; attr++) {
            switch (layout.Components[attr].Format) {
                case VertexFormat::Float4:
                    {
                        const float* fPtr = (const float*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*fPtr++));
                        ptr = (const uint8_t*) fPtr;
                    }
                    // fallthrough!
                case VertexFormat::Float3:
                    {
                        const float* fPtr = (const float*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*fPtr++));
                        ptr = (const uint8_t*) fPtr;
                    }
                    // fallthrough!
                case VertexFormat::Float2:
                    {
                        const float* fPtr = (const float*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*fPtr++));
                        ptr = (const uint8_t*) fPtr;
                    }
                    // fallthrough!
                case VertexFormat::Float:
                    {
                        const float* fPtr = (const float*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*fPtr++));
                        ptr = (const uint8_t*) fPtr;
                    }
                    break;
                case VertexFormat::Byte4:
                case VertexFormat::Byte4N:
                case VertexFormat::UByte4:
                case VertexFormat::UByte4N:
                    {
                        const uint32_t* ui32Ptr = (const uint32_t*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*ui32Ptr++));
                        ptr = (const uint8_t*) ui32Ptr;
                    }
                    break;
                case VertexFormat::Short4:
                case VertexFormat::Short4N:
                    {
                        const uint16_t* ui16Ptr = (const uint16_t*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*ui16Ptr++));
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*ui16Ptr++));
                        ptr = (const uint8_t*) ui16Ptr;
                    }
                    // fallthrough!
                case VertexFormat::Short2:
                case VertexFormat::Short2N:
                    {
                        const uint16_t* ui16Ptr = (const uint16_t*) ptr;
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*ui16Ptr++));
                        cJSON_AddItemToArray(jsonVertex, cJSON_CreateNumber(*ui16Ptr++));
                        ptr = (const uint8_t*) ui16Ptr;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void
JSONMeshDumper::dumpIndices(cJSON* jsonNode, const Mesh& mesh) {

    cJSON* jsonIndices = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonNode, "indices", jsonIndices);
    const int numIndices = mesh.IndexBuffer.GetNumIndices();
    if (2 == mesh.IndexBuffer.GetIndexSize()) {
        // 16-bit indices
        const uint16_t* ptr = (const uint16_t*) mesh.IndexBuffer.GetDataPointer();
        for (int i = 0; i < numIndices; i++) {
            cJSON_AddItemToArray(jsonIndices, cJSON_CreateNumber(*ptr++));
        }
    }
    else if (4 == mesh.IndexBuffer.GetIndexSize()) {
        // 32-bit indices
        const uint32_t* ptr = (const uint32_t*) mesh.IndexBuffer.GetDataPointer();
        for (int i = 0; i < numIndices; i++) {
            cJSON_AddItemToArray(jsonIndices, cJSON_CreateNumber(*ptr++));
        }
    }
    else {
        Log::Fatal("JSONMeshDumper::dumpIndices(): invalid index size\n");
    }
}

} // namespace OryolTools