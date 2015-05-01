#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::IndexBuffer
    @brief holds exported index data
*/
#include <cstdint>

namespace OryolTools {

class IndexBuffer {
public:
    /// destructor
    ~IndexBuffer();

    /// setup the object, indexSize must be 2 or 4 (16-bit or 32-bit indices)
    void Setup(int indexSize, int numIndices);
    /// discard the object
    void Discard();
    /// return true if object has been setup
    bool IsValid() const;

    /// write a number of indices
    void Write(int startIndex, const unsigned int* input, int numIndices, unsigned int baseVertexIndex);

    /// get index size (2 or 4)
    int GetIndexSize() const;
    /// get number of indices
    int GetNumIndices() const;
    /// get pointer to index data
    const std::uint8_t* GetDataPointer() const;
    /// get size of index data in bytes
    int GetDataSize() const;

private:
    int indexSize = 0;
    int allNumIndices = 0;
    std::uint8_t* buffer = nullptr;
};

} // namespace OryolTools