#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::PrimitiveGroup
    @brief a group of primitives in a mesh (aka submesh)
*/
namespace OryolTools {

class PrimitiveGroup {
public:
    enum TypeT {
        Points = 0,
        Lines,
        LineLoop,
        LineStrip,
        Triangles,
        TriangleStrip,
        TriangleFan,

        NumTypes,
        InvalidType,
    } Type = Triangles;
    int BaseElement = 0;
    int NumElements = 0;

    static std::string TypeToString(TypeT t) {
        static const std::string types[NumTypes] = {
            "Points",
            "Lines",
            "LineLoop",
            "LineStrip",
            "Triangles",
            "TriangleStrip",
            "TriangleFan",
        };
        assert(t < NumTypes);
        return types[t];
    };
};

} // namespace OryolTools
