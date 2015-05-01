//------------------------------------------------------------------------------
//  IndexBuffer.cc
//------------------------------------------------------------------------------
#include "IndexBuffer.h"
#include "ExportUtil/Log.h"
#include <cassert>

using namespace std;

namespace OryolTools {

//------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer() {
    if (this->IsValid()) {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
void
IndexBuffer::Setup(int idxSize, int numIndices) {
    assert(!this->IsValid());
    assert((2 == idxSize) || (4 == idxSize));
    assert(0 < numIndices);

    this->indexSize = idxSize;
    this->allNumIndices = numIndices;
    this->buffer = (uint8_t*) calloc(this->allNumIndices, this->indexSize);
}

//------------------------------------------------------------------------------
void
IndexBuffer::Discard() {
    assert(this->IsValid());
    free(this->buffer);
    this->buffer = nullptr;
    this->allNumIndices = 0;
    this->indexSize = 0;
}

//------------------------------------------------------------------------------
bool
IndexBuffer::IsValid() const {
    return nullptr != this->buffer;
}

//------------------------------------------------------------------------------
int
IndexBuffer::GetIndexSize() const {
    assert(this->IsValid());
    return this->indexSize;
}

//------------------------------------------------------------------------------
int
IndexBuffer::GetNumIndices() const {
    assert(this->IsValid());
    return this->allNumIndices;
}

//------------------------------------------------------------------------------
const uint8_t*
IndexBuffer::GetDataPointer() const {
    assert(this->IsValid());
    return this->buffer;
}

//------------------------------------------------------------------------------
int
IndexBuffer::GetDataSize() const {
    assert(this->IsValid());
    return this->allNumIndices * this->indexSize;
}

//------------------------------------------------------------------------------
void
IndexBuffer::Write(int startIndex, const unsigned int* input, int numIndices, unsigned int baseVertexIndex) {
    assert(this->IsValid());
    assert((startIndex + numIndices) <= this->allNumIndices);
    if (2 == this->indexSize) {
        uint16_t* ptr = ((uint16_t*)this->buffer) + startIndex;
        for (int i = 0; i < numIndices; i++) {
            unsigned int index = input[i] + baseVertexIndex;
            if (index >= (1<<16)) {
                Log::Fatal("16-bit indices requested, but found index >= (1<<16)!\n");
                break;
            }
            ptr[i] = (uint16_t) index;
        }
    }
    else {
        assert(4 == this->indexSize);
        uint32_t* ptr = ((uint32_t*)this->buffer) + startIndex;
        for (int i = 0; i < numIndices; i++) {
            unsigned int index = input[i] + baseVertexIndex;
            ptr[i] = index;
        }
    }
}

} // namespace OryolTools