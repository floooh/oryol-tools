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
    /// the intended vertex packing format
    std::map<VertexAttr::Code, VertexFormat::Code> VertexFormats;
    /// save IRep to ORB
    void Save(const std::string& path, const IRep& irep);

    VertexFormat::Code mapVertexFormat(VertexAttr::Code attr, VertexFormat::Code fmt) const;
    int vertexStrideInBytes(const IRep& irep) const;
    uint32_t addString(const std::string& str);

    std::vector<std::string> strings;
};