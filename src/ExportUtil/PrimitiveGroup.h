#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::PrimitiveGroup
    @brief a group of primitives in a mesh (aka submesh)
*/
namespace OryolTools {

class PrimitiveGroup {
public:
    enum {
        Points = 0,
        Lines = 1,
        LineLoop = 2,
        LineStrip = 3,
        Triangles = 4,
        TriangleStrip = 5,
        TriangleFan = 6
    } Type = Triangles;
    int BaseElement = 0;
    int NumElements = 0;
};

} // namespace OryolTools
