//------------------------------------------------------------------------------
//  VertexWriter.cc
//------------------------------------------------------------------------------
#include "VertexWriter.h"
#include "glm/glm.hpp"

using namespace std;

namespace OryolTools {

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Float>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    float* p = (float*) dst;
    *p = *src * scale;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Float2>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    float* p = (float*) dst;
    *p++ = x; *p++ = y;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Float3>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    float* p = (float*) dst;
    *p++ = x; *p++ = y; *p++ = z;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Float4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    float* p = (float*) dst;
    *p++ = x; *p++ = y; *p++ = z; *p++ = w;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Byte4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i8vec4 packed(glm::clamp(glm::vec4(x, y, z, w), -128.0f, 127.0f));
    int8_t* p = (int8_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Byte4N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i8vec4 packed(glm::round(glm::clamp(glm::vec4(x, y, z, w), -1.0f, 1.0f) * 127.0f));
    int8_t* p = (int8_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::UByte4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::u8vec4 packed(glm::clamp(glm::vec4(x, y, z, w), 0.0f, 255.0f));
    *dst++ = packed.x; *dst++ = packed.y; *dst++ = packed.z; *dst++ = packed.w;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::UByte4N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::u8vec4 packed(glm::round(glm::clamp(glm::vec4(x, y, z, w), 0.0f, 1.0f) * 255.0f));
    *dst++ = packed.x; *dst++ = packed.y; *dst++ = packed.z; *dst++ = packed.w;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Short2>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    glm::i16vec2 packed(glm::clamp(glm::vec2(x, y), -32768.0f, 32767.0f));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Short2N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    glm::i16vec2 packed(glm::round(glm::clamp(glm::vec2(x, y), -1.0f, 1.0f) * 32767.0f));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x;
    *p++ = packed.y;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Short4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i16vec4 packed(glm::clamp(glm::vec4(x, y, z, w), -32768.0f, 32767.0f));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
}

//------------------------------------------------------------------------------
template<> void
VertexWriter::Write<VertexFormat::Short4N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i16vec4 packed(glm::round(glm::clamp(glm::vec4(x, y, z, w), -1.0f, 1.0f) * 32767.0f));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
}

} // namespace OryolTools
