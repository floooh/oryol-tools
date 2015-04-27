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
    };
    static Code FromString(const std::string& str) {
        for (int i = 0; i < NumVertexFormats; i++) {
            if (ToString((Code)i) == str) {
                return (Code)i;
            }
        }
        return InvalidVertexFormat;
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
        VertexFormat::Code format = VertexFormat::InvalidVertexFormat;
        float scale = 1.0f;
        float bias = 0.0f;
    } Components[VertexAttr::NumVertexAttrs];
};

} // namespace OryolTools