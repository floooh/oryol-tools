//------------------------------------------------------------------------------
//  VertexBuffer.cc
//------------------------------------------------------------------------------
#include "VertexBuffer.h"
#include "ExportUtil/VertexCodec.h"
#include <cstdlib>

using namespace std;

//------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer() {
    if (this->IsValid()) {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
void
VertexBuffer::Setup(const VertexLayout& vertexLayout, int numVerts) {
    assert(numVerts > 0);
    assert(nullptr == this->buffer);

    this->layout = vertexLayout;
    this->allNumVertices = numVerts;
    this->buffer = (uint8_t*) calloc(this->allNumVertices, this->layout.ByteSize());
}

//------------------------------------------------------------------------------
void
VertexBuffer::Discard() {
    assert(this->IsValid());
    
    free(this->buffer);
    this->buffer = nullptr;
    this->allNumVertices = 0;
}

//------------------------------------------------------------------------------
bool
VertexBuffer::IsValid() const {
    return nullptr != this->buffer;
}

//------------------------------------------------------------------------------
const VertexLayout&
VertexBuffer::GetVertexLayout() const {
    assert(this->IsValid());
    return this->layout;
}

//------------------------------------------------------------------------------
int
VertexBuffer::GetNumVertices() const {
    assert(this->IsValid());
    assert(this->allNumVertices > 0);
    return this->allNumVertices;
}

//------------------------------------------------------------------------------
const uint8_t*
VertexBuffer::GetDataPointer() const {
    assert(this->IsValid());
    return this->buffer;
}

//------------------------------------------------------------------------------
int
VertexBuffer::GetDataSize() const {
    assert(this->IsValid());
    assert(this->allNumVertices > 0);
    const int size = this->allNumVertices * this->layout.ByteSize();
    assert(size > 0);
    return size;
}

//------------------------------------------------------------------------------
template<VertexFormat::Code FORMAT> void
VertexBuffer::write(VertexAttr::Code attr,
                    uint8_t* dstPtr,
                    int numVerts,
                    const float* input,
                    int numInputComps,
                    int inputStride)
{
    const int dstStride = this->layout.ByteSize();
    const glm::vec4 scale(this->layout.Components[attr].Scale);
    for (int i = 0; i < numVerts; i++) {
        VertexCodec::Encode<FORMAT>(dstPtr, scale, input, numInputComps);
        dstPtr += dstStride;
        input += inputStride;
    }
}

//------------------------------------------------------------------------------
void
VertexBuffer::Write(VertexAttr::Code attr,
                    int startVertexIndex,
                    int numVertices,
                    const float* input,
                    int numInputComps,        // 1, 2, 3 or 4 input components
                    int inputStride)          // distance between input comps, in number of floats
{
    assert(this->IsValid());
    assert((startVertexIndex >= 0) && (numVertices > 0));
    assert((startVertexIndex + numVertices) <= this->allNumVertices);
    assert(nullptr != input);
    assert((numInputComps >= 1) && (numInputComps <= 4));
    assert(inputStride > 0);
    assert(VertexFormat::Invalid != this->layout.Components[attr].Format);

    const int vertexByteSize = this->layout.ByteSize();
    const int compOffset = this->layout.Offset(attr);
    uint8_t* ptr = this->buffer + startVertexIndex * vertexByteSize + compOffset;

    switch (this->layout.Components[attr].Format) {
        case VertexFormat::Float:
            this->write<VertexFormat::Float>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Float2:
            this->write<VertexFormat::Float2>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Float3:
            this->write<VertexFormat::Float3>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Float4:
            this->write<VertexFormat::Float4>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Byte4:
            this->write<VertexFormat::Byte4>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Byte4N:
            this->write<VertexFormat::Byte4N>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::UByte4:
            this->write<VertexFormat::UByte4>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::UByte4N:
            this->write<VertexFormat::UByte4N>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Short2:
            this->write<VertexFormat::Short2>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Short2N:
            this->write<VertexFormat::Short2N>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Short4:
            this->write<VertexFormat::Short4>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        case VertexFormat::Short4N:
            this->write<VertexFormat::Short4N>(attr, ptr, numVertices, input, numInputComps, inputStride);
            break;
        default:
            break;
    }
}
