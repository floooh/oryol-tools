#pragma once
//------------------------------------------------------------------------------
/**
    @class OrbSaver
    @brief save intermediate representation to ORB (ORyol Binary format)
*/
#include <string>
#include <vector>
#include <map>
#include "ExportUtil/Vertex.h"
#include "IRep.h"
#include "OrbFileFormat.h"

struct OrbSaver {
    /// the requested vertex layout, drop any src components not in here, ignore non-existing src comps
    VertexLayout Layout;
    /// this is the cross-section of the requested layout, and the IRep layout
    VertexLayout DstLayout;
    /// save IRep to ORB
    void Save(const std::string& path, const IRep& irep);

    uint32_t addString(const std::string& str);

    std::vector<std::string> strings;
};
