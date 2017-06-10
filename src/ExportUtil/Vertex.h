#pragma once
//------------------------------------------------------------------------------
#include <string>
#include <cassert>
#include <vector>
#include <initializer_list>

//------------------------------------------------------------------------------
/**
    @class VertexAttr
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

        Num,
        Invalid,
    };
    static const char* ToString(Code c) {
        switch (c) {
            case Position: return "Position";
            case Normal: return "Normal";
            case TexCoord0: return "TexCoord0";
            case TexCoord1: return "TexCoord1";
            case TexCoord2: return "TexCoord2";
            case TexCoord3: return "TexCoord3";
            case Tangent: return "Tangent";
            case Binormal: return "Binormal";
            case Weights: return "Weights";
            case Indices: return "Indices";
            case Color0: return "Color0";
            case Color1: return "Color1";
            default: return "Invalid";
        };
    };
    static Code FromString(const std::string& str) {
        for (int i = 0; i < Num; i++) {
            if (str == ToString((Code)i)) {
                return (Code) i;
            }
        }
        return Invalid;
    };
};

//------------------------------------------------------------------------------
/**
    @class VertexFormat
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

        Num,
        Invalid,
    };
    static const char* ToString(Code c) {
        switch (c) {
            case Float: return "Float";
            case Float2: return "Float2";
            case Float3: return "Float3";
            case Float4: return "Float4";
            case Byte4: return "Byte4";
            case Byte4N: return "Byte4N";
            case UByte4: return "UByte4";
            case UByte4N: return "UByte4N";
            case Short2: return "Short2";
            case Short2N: return "Short2N";
            case Short4: return "Short4";
            case Short4N: return "Short4N";
            default: return "Invalid";
        };
    }
    /// convert from string
    static Code FromString(const std::string& str) {
        for (int i = 0; i < Num; i++) {
            if (str == ToString((Code)i)) {
                return (Code)i;
            }
        }
        return Invalid;
    }
    /// get byte size
    static int ByteSize(VertexFormat::Code c) {
        switch (c) {
            case Float: 
            case Byte4:
            case Byte4N:
            case UByte4:
            case UByte4N:
            case Short2:
            case Short2N:
                return 4;
            case Float2:
            case Short4:
            case Short4N:
                return 8;
            case Float3:
                return 12;
            case Float4:
                return 16;
            default:
                return 0;
        }
    }
    /// check if this is a packed format (not one of the float formats)
    static bool IsPacked(VertexFormat::Code c) {
        switch (c) {
            case Float:
            case Float2:
            case Float3:
            case Float4:
                return false;
            case Byte4:
            case Byte4N:
            case UByte4:
            case UByte4N:
            case Short2:
            case Short2N:
            case Short4:
            case Short4N:
            default:
                return true;
        }
    }
    /// number of items in the vertex format
    static int NumItems(VertexFormat::Code c) {
        switch (c) {
            case Float:
                return 1;
            case Float2:
            case Short2:
            case Short2N:
                return 2;
            case Float3:
                return 3;
            case Float4:
            case Byte4:
            case Byte4N:
            case UByte4:
            case UByte4N:
            case Short4:
            case Short4N:
                return 4;
            default:
                return 0;
        }
    }
};

//------------------------------------------------------------------------------
struct VertexComponent {
    VertexComponent(VertexAttr::Code attr, VertexFormat::Code fmt=VertexFormat::Invalid, float scale=1.0f, float bias=0.0f):
        Attr(attr), Format(fmt), Scale(scale), Bias(bias) { };
    VertexAttr::Code Attr = VertexAttr::Invalid;
    VertexFormat::Code Format = VertexFormat::Invalid;
    float Scale = 1.0f;
    float Bias = 0.0f;
};

//------------------------------------------------------------------------------
/**
    @class OryolTools::VertexLayout
    @brief vertex component layout
*/
class VertexLayout {
public:
    std::vector<VertexComponent> Components;

    /// default constructor
    VertexLayout() { };
    /// initialize from a component array
    VertexLayout(std::initializer_list<VertexComponent> l):
        Components(l) { };

    /// test if the vertex layout has a vertex attribute
    bool HasAttr(VertexAttr::Code attr) const {
        for (const auto& comp : this->Components) {
            if (comp.Attr == attr) {
                return true;
            }
        }
        return false;
    }

    /// get format by attr, return VertexFormat::Invalid if not found
    VertexFormat::Code AttrFormat(VertexAttr::Code attr) const {
        for (const auto& comp : this->Components) {
            if (comp.Attr == attr) {
                return comp.Format;
            }
        }
        return VertexFormat::Invalid;
    }

    /// compute byte size
    int ByteSize() const {
        int size = 0;
        for (const auto& comp : this->Components) {
            size += VertexFormat::ByteSize(comp.Format);
        }
        return size;
    }

    /// get byte-offset of attr
    int Offset(VertexAttr::Code attr) const {
        int offset = 0;
        for (const auto& comp : this->Components) {
            if (comp.Attr == attr) {
                break;
            }
            offset += VertexFormat::ByteSize(comp.Format);
        }
        return offset;
    }
};
