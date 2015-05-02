//------------------------------------------------------------------------------
//  MeshSaver.cc
//------------------------------------------------------------------------------
#include "MeshSaver.h"

using namespace std;

namespace OryolTools {

//------------------------------------------------------------------------------
size_t
MeshSaver::write32(FILE* fp, uint32_t val) {
    return fwrite(&val, 1, sizeof(val), fp);
}

//------------------------------------------------------------------------------
size_t
MeshSaver::Save(const Mesh& mesh, FILE* fp) {
    assert(nullptr != fp);
    assert(mesh.VertexBuffer.IsValid());
    assert(mesh.IndexBuffer.IsValid());
    assert(!mesh.PrimGroups.empty());

    const VertexLayout& layout = mesh.VertexBuffer.GetVertexLayout();
    size_t size = 0;

    // write magic number and header data
    size += write32(fp, 'OMSH');
    size += write32(fp, mesh.VertexBuffer.GetNumVertices());
    size += write32(fp, layout.ByteSize());
    size += write32(fp, mesh.IndexBuffer.GetNumIndices());
    size += write32(fp, mesh.IndexBuffer.GetIndexSize());
    size += write32(fp, layout.NumValidComponents());
    size += write32(fp, mesh.PrimGroups.size());

    // write vertex layout
    for (uint32_t attr = 0; attr < VertexAttr::NumVertexAttrs; attr++) {
        const auto& comp = layout.Components[attr];
        if (VertexFormat::InvalidVertexFormat != comp.Format) {
            size += write32(fp, attr);
            size += write32(fp, comp.Format);
        }
    }

    // write primitive groups
    for (const auto& primGroup : mesh.PrimGroups) {
        size += write32(fp, primGroup.Type);
        size += write32(fp, primGroup.BaseElement);
        size += write32(fp, primGroup.NumElements);
    }

    // write vertices
    const void* vertexDataPtr = mesh.VertexBuffer.GetDataPointer();
    const size_t vertexDataSize = mesh.VertexBuffer.GetDataSize();
    size += fwrite(vertexDataPtr, 1, vertexDataSize, fp);

    // write indices
    const void* indexDataPtr = mesh.IndexBuffer.GetDataPointer();
    const size_t indexDataSize = mesh.IndexBuffer.GetDataSize();
    size += fwrite(indexDataPtr, 1, indexDataSize, fp);

    // write padding if needed
    if ((size & 3) != 0) {
        uint16_t padding = 0;
        size += fwrite(&padding, 1, sizeof(padding), fp);
        assert((size & 3) == 0);
    }

    return size;
}

} // namespace OryolTools
