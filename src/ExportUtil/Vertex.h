#pragma once
//------------------------------------------------------------------------------
#include <string>
#include <cassert>

namespace OryolTools {

//------------------------------------------------------------------------------
/**
    @class OryolTool::VertexAttr
    @brief vertex attribute enum
*/
class VertexAttr {
public:
    enum Code {
        Position = 0,
        Normal,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        Tangent,
        Binormal,
        Weights,
        Indices,
        Color0,
        Color1,

        NumVertexAttrs,
        InvalidVertexAttr
    };
    static const std::string& ToString(Code c) {
        static const std::string names[NumVertexAttrs] = {
            "Position",
            "Normal",
            "TexCoord0",
            "TexCoord1",
            "TexCoord2",
            "TexCoord3",
            "Tangent",
            "Binormal",
            "Weights",
            "Indices",
            "Color0",
            "Color1"
        };
        assert(c < NumVertexAttrs);
        return names[c];
    };
    static Code FromString(const std::string& str) {
        for (int i = 0; i < NumVertexAttrs; i++) {
            if (ToString((Code)i) == str) {
                return (Code) i;
            }
        }
        return InvalidVertexAttr;
    };
};

//------------------------------------------------------------------------------
/**
    @class OryolTools::VertexFormat
    @brief vertex format enum
*/
class VertexFormat {
public:
    enum Code {
        Float = 0,
        Float2,
        Float3,
        Float4,
        Byte4,
        Byte4N,
        UByte4,
        UByte4N,
        Short2,
        Short2N,
        Short4,
        Short4N,

        NumVertexFormats,
        InvalidVertexFormat,
    };
    static const std::string& ToString(Code c) {
        static const std::string names[NumVertexFormats] = {
            "Float",
            "Float2",
            "Float3",
            "Float4",
            "Byte4",
            "Byte4N",
            "UByte4",
            "UByte4N",
            "Short2",
            "Short2N",
            "Short4",
            "Short4N"
        };
        assert(c < NumVertexFormats);
        return names[c];
    }
    /// convert from string
    static Code FromString(const std::string& str) {
        for (int i = 0; i < NumVertexFormats; i++) {
            if (ToString((Code)i) == str) {
                return (Code)i;
            }
        }
        return InvalidVertexFormat;
    }
    /// get byte size
    static int ByteSize(VertexFormat::Code c) {
        static const int sizes[NumVertexFormats] = {
            4,      // Float
            8,      // Float2
            12,     // Float3
            16,     // Float4
            4,      // Byte4
            4,      // Byte4N
            4,      // UByte4
            4,      // UByte4N
            4,      // Short2
            4,      // Short2N
            8,      // Short4
            8,      // Short4N
        };
        assert((c >= 0) && (c < NumVertexFormats));
        return sizes[c];
    }
};

//------------------------------------------------------------------------------
/**
    @class OryolTools::VertexLayout
    @brief vertex component layout
*/
class VertexLayout {
public:
    struct Component {
        VertexFormat::Code Format = VertexFormat::InvalidVertexFormat;
        float Scale = 1.0f;
        float Bias = 0.0f;
    } Components[VertexAttr::NumVertexAttrs];

    /// test if the vertex layout has a vertex attribute
    bool HasAttr(VertexAttr::Code c) const {
        assert((c >= 0) && (c < VertexAttr::NumVertexAttrs));
        return this->Components[c].Format != VertexFormat::InvalidVertexFormat;
    }

    /// get number of valid vertex components
    int NumValidComponents() const {
        int num = 0;
        for (const auto& c : this->Components) {
            if (c.Format != VertexFormat::InvalidVertexFormat) {
                num++;
            }
        }
        return num;
    }

    /// compute byte size
    int ByteSize() const {
        int size = 0;
        for (const auto& c : this->Components) {
            if (c.Format != VertexFormat::InvalidVertexFormat) {
                size += VertexFormat::ByteSize(c.Format);
            }
        }
        return size;
    }

    /// get byte-offset of attr
    int Offset(VertexAttr::Code attr) const {
        int offset = 0;
        for (int i = 0; i < VertexAttr::NumVertexAttrs; i++) {
            if (i < attr) {
                if (this->Components[i].Format != VertexFormat::InvalidVertexFormat) {
                    offset += VertexFormat::ByteSize(this->Components[i].Format);
                }
            }
            else {
                break;
            }
        }
        return offset;
    }
};

} // namespace OryolTools