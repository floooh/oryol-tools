#pragma once
//------------------------------------------------------------------------------
/**
    @class IRepProcessor
    @brief perform actions on IRep based on JSON rule file
*/
#include "IRep.h"
#include <vector>
#include <string>

struct IRepProcessor {
    /// if not empty, non-matching nodes will be dropped
    std::vector<std::string> Nodes;
    /// if not empty, non-matching anim clips will be dropped
    std::vector<std::string> Clips;

    /// reset processor into its empty state
    void Clear();
    /// load preprocessor state from JSON
    void Load(const std::string& path);
    /// process an IRep
    void Process(IRep& irep);

    /// remove anim clips
    void RemoveClips(IRep& irep, const std::vector<std::string>& clipNames);
    /// remove nodes, meshes and orphaned materials
    void RemoveNodes(IRep& irep, const std::vector<std::string>& nodeNames);
    /// remove a vertex range and fix meshes
    void RemoveVertices(IRep& irep, int first, int num);
    /// remove an index range and fix meshes
    void RemoveIndices(IRep& irep, int first, int num);    
};
