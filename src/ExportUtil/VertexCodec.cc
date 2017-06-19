//------------------------------------------------------------------------------
//  VertexCodec.cc
//------------------------------------------------------------------------------
#include "VertexCodec.h"
#include "glm/glm.hpp"

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Float>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    float* p = (float*) dst;
    *p++ = *src * scale;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Float>(float* dst, float scale, float bias, const uint8_t* src, int /*numSrcComps*/, int /*numDstComps*/) {
    const float* p = (const float*) src;
    *dst = (*p * scale) + bias;
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Float2>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    float* p = (float*) dst;
    for (int i = 0; i < 2; i++) {
        *p++ = (numSrcComps > i) ? src[i] * scale : 0.0f;
    }
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Float2>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const float* p = (const float*) src;
    for (int i = 0; i < 2; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? (p[i] * scale) + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Float3>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    float* p = (float*) dst;
    for (int i = 0; i < 3; i++) {
        *p++ = (numSrcComps > i) ? src[i] * scale : 0.0f;
    }
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Float3>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const float* p = (const float*) src;
    for (int i = 0; i < 3; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? p[i] * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Float4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    float* p = (float*) dst;
    for (int i = 0; i < 4; i++) {
        *p++ = (numSrcComps > i) ? src[i] * scale : 0.0f;
    }
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Float4>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const float* p = (const float*) src;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? p[i] * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Byte4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i8vec4 packed(glm::clamp(glm::vec4(x, y, z, w), -128.0f, 127.0f));
    int8_t* p = (int8_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Byte4>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const int8_t* p = (const int8_t*) src;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Byte4N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    scale *= 127.0f;
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i8vec4 packed(glm::round(glm::clamp(glm::vec4(x, y, z, w), -128.0f, 127.0f)));
    int8_t* p = (int8_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Byte4N>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    scale /= 127.0f;
    const int8_t* p = (const int8_t*) src;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::UByte4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::u8vec4 packed(glm::clamp(glm::vec4(x, y, z, w), 0.0f, 255.0f));
    *dst++ = packed.x; *dst++ = packed.y; *dst++ = packed.z; *dst++ = packed.w;
    return dst;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::UByte4>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const uint8_t* p = (const uint8_t*) src;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::UByte4N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    scale *= 255.0f;
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::u8vec4 packed(glm::round(glm::clamp(glm::vec4(x, y, z, w), 0.0f, 255.0f)));
    *dst++ = packed.x; *dst++ = packed.y; *dst++ = packed.z; *dst++ = packed.w;
    return dst;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::UByte4N>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    scale /= 255.0f;
    const uint8_t* p = (const uint8_t*) src;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Short2>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    glm::i16vec2 packed(glm::clamp(glm::vec2(x, y), -32768.0f, 32767.0f));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Short2>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const int16_t* p = (const int16_t*) src;
    for (int i = 0; i < 2; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Short2N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    scale *= 32767.0f;
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    glm::i16vec2 packed(glm::round(glm::clamp(glm::vec2(x, y), -32768.0f, 32767.0f)));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Short2N>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    scale /= 32767.0f;
    const int16_t* p = (const int16_t*) src;
    for (int i = 0; i < 2; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Short4>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i16vec4 packed(glm::clamp(glm::vec4(x, y, z, w), -32768.0f, 32767.0f));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Short4>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    const int16_t* p = (const int16_t*) dst;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
template<> uint8_t*
VertexCodec::Encode<VertexFormat::Short4N>(uint8_t* dst, float scale, const float* src, int numSrcComps) {
    scale *= 32767.0f;
    const float x = src[0] * scale;
    const float y = (numSrcComps > 1) ? src[1] * scale : 0.0f;
    const float z = (numSrcComps > 2) ? src[2] * scale : 0.0f;
    const float w = (numSrcComps > 3) ? src[3] * scale : 0.0f;
    glm::i16vec4 packed(glm::round(glm::clamp(glm::vec4(x, y, z, w), -32768.0f, 32767.0f)));
    int16_t* p = (int16_t*) dst;
    *p++ = packed.x; *p++ = packed.y; *p++ = packed.z; *p++ = packed.w;
    return (uint8_t*)p;
}

//------------------------------------------------------------------------------
template<> void
VertexCodec::Decode<VertexFormat::Short4N>(float* dst, float scale, float bias, const uint8_t* src, int numSrcComps, int numDstComps) {
    scale /= 32767.0f;
    const int16_t* p = (const int16_t*) dst;
    for (int i = 0; i < 4; i++) {
        if (i < numDstComps) {
            *dst++ = (numSrcComps > i) ? float(p[i]) * scale + bias : 0.0f;
        }
    }
}
