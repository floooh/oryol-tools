//------------------------------------------------------------------------------
//  IRepProcessor.cc
//------------------------------------------------------------------------------
#include "IRepProcessor.h"
#include "LoadUtil.h"
#include "cJSON.h"
extern "C" {
#include "cJSON_Utils.h"
}

using namespace std;

//------------------------------------------------------------------------------
void
IRepProcessor::Clear() {
    this->Nodes.clear();
    this->Clips.clear();
}

//------------------------------------------------------------------------------
static void parseStringArray(const char* path, cJSON* arrayNode, vector<string>& dst) {
    Log::FailIf(!cJSON_IsArray(arrayNode), "JSON '%s' must be an array of strings\n", path);
    for (int i = 0; i < cJSON_GetArraySize(arrayNode); i++) {
        cJSON* str = cJSON_GetArrayItem(arrayNode, i);
        Log::FailIf(!cJSON_IsString(str), "JSON filter items must be strings\n");
        dst.push_back(str->valuestring);
    }
}

//------------------------------------------------------------------------------
void
IRepProcessor::Load(const string& path) {
    this->Clear();

    // load file as zero-terminated string
    const char* str = (const char*) load_file(path);
    cJSON* json = cJSON_Parse(str);
    Log::FailIf(!json, "Failed to parse JSON file '%s' with '%s'\n", path.c_str(), cJSON_GetErrorPtr()); 
    cJSON* node;
    if ((node = cJSONUtils_GetPointer(json, "/filter/nodes"))) {
        parseStringArray("/filter/nodes", node, this->Nodes);
    }
    if ((node = cJSONUtils_GetPointer(json, "/filter/clips"))) {
        parseStringArray("/filter/clips", node, this->Clips);
    }
}

//------------------------------------------------------------------------------
static bool contains(const vector<string>& items, const string& item) {
    return (find(items.begin(), items.end(), item) != items.end());
}

//------------------------------------------------------------------------------
static vector<string> 
matchItems(const vector<string>& allItems, const vector<string>& filterItems) {
    // return a string vector with all items not in filter-items
    vector<string> res;
    for (const auto& item : allItems) {
        if (!contains(filterItems, item)) {
            res.push_back(item);
        }
    }
    return res;
}

//------------------------------------------------------------------------------
void
IRepProcessor::Process(IRep& irep) {
    // need to remove nodes?
    if (!this->Nodes.empty()) {
        this->RemoveNodes(irep, matchItems(irep.NodeNames(), this->Nodes));
    }

    // need to remove clips?
    if (!this->Clips.empty()) {
        this->RemoveClips(irep, matchItems(irep.ClipNames(), this->Clips));
    }
}

//------------------------------------------------------------------------------
static void markInvalidNode(IRep& irep, int nodeIndex) {
    irep.Nodes[nodeIndex].Parent = -2;
    for (int i = nodeIndex; i < int(irep.Nodes.size()); i++) {
        if (irep.Nodes[i].Parent == nodeIndex) {
            markInvalidNode(irep, i);
        }
    }
}

//------------------------------------------------------------------------------
void
IRepProcessor::RemoveNodes(IRep& irep, const vector<string>& nodeNames) {

    // mark all nodes to remove with a special Parent index of -2
    for (const auto& name : nodeNames) {
        for (int i = 0; i < int(irep.Nodes.size()); i++) {
            if (name == irep.Nodes[i].Name) {
                // this is the node to remove, set it's parent index to -2 (this
                // is the magic marker that the node is invalid), and also
                // find all child nodes and mark them as invalid
                markInvalidNode(irep, i);
            }
        }
    }

    // fix up the parent indices of the remaining nodes, not the fastest
    // way to do this, but easy to understand...
    for (int i = 0; i < int(irep.Nodes.size()); i++) {
        if (irep.Nodes[i].Parent == -2) {
            for (int j = i; j < int(irep.Nodes.size()); j++) {
                if (irep.Nodes[j].Parent > i) {
                    irep.Nodes[j].Parent--;
                    Log::FailIf(irep.Nodes[j].Parent < 0, "IRepProcessor::RemoveNodes: negative index error when fixing parent indices\n");
                }
            }
        }
    }

    // remove all orphaned vertices and indices, and fix up the
    // vertex/index indices in remaining meshes
    const int stride = irep.VertexStrideBytes() / sizeof(float);
    for (const auto& n0 : irep.Nodes) {
        if (n0.Parent == -2) {
            for (const auto& m0 : n0.Meshes) {
                const uint32_t vi0 = m0.FirstVertex * stride;
                const uint32_t vi1 = vi0 + m0.NumVertices * stride;
                Log::FailIf(vi1 > irep.VertexData.size(), "IRepProcessor::RemoveNodes(): vertex range removal overflow!\n");
                irep.VertexData.erase(irep.VertexData.begin() + vi0, irep.VertexData.begin() + vi1);
                const uint32_t ii0 = m0.FirstIndex;
                const uint32_t ii1 = ii0 + m0.NumIndices;
                Log::FailIf(ii1 > irep.IndexData.size(), "IRepProcessor::RemoveNodes(): index range removal overflow!\n");
                irep.IndexData.erase(irep.IndexData.begin() + ii0, irep.IndexData.begin() + ii1);

                // fixup vertex/index indices in remaining meshes
                for (auto& n1 : irep.Nodes) {
                    for (auto& m1 : n1.Meshes) {
                        if (m1.FirstVertex > m0.FirstVertex) {
                            m1.FirstVertex -= m0.NumVertices;
                            Log::FailIf((m1.FirstVertex+m1.NumVertices)*stride > irep.VertexData.size(),
                                "IRepProcessor::RemoveNodes(): vertex range fixup overflow error!\n");
                        }
                        if (m1.FirstIndex > m0.FirstIndex) {
                            m1.FirstIndex -= m0.NumIndices;
                            Log::FailIf((m1.FirstIndex+m1.NumIndices) > irep.IndexData.size(),
                                "IRepProcessor::RemoveNodes(): index range fixup overflow error!\n");
                        }
                    }
                }

                // fixup remaining indices
                const uint32_t numVertices = irep.VertexData.size() / stride;
                for (uint16_t& vi : irep.IndexData) {
                    if (vi > m0.FirstIndex) {
                        vi -= m0.NumVertices;
                        Log::FailIf(vi >= numVertices, "IRepProcessor::RemoveNodes: index fixup overflow error!\n");
                    }
                }
            }
        }
    }

    // can now remove all nodes marked for removal
    for (auto iter = irep.Nodes.begin(); iter != irep.Nodes.end();) {
        if (-2 == iter->Parent) {
            irep.Nodes.erase(iter);
        }
        else {
            iter++;
        }
    }

    // remove all orphaned materials
    for (uint32_t matIndex = 0; matIndex < irep.Materials.size();) {
        bool isOrphaned = true;
        for (const auto& node : irep.Nodes) {
            for (const auto& mesh : node.Meshes) {
                if (mesh.Material == matIndex) {
                    isOrphaned = false;
                    break;
                }
            }
        }
        if (isOrphaned) {
            irep.Materials.erase(irep.Materials.begin() + matIndex);
            // fixup material indices in meshes
            for (auto& node : irep.Nodes) {
                for (auto& mesh : node.Meshes) {
                    if (mesh.Material > matIndex) {
                        mesh.Material--;
                    }
                }
            }
        }
        else {
            matIndex++;
        }
    }
}

//------------------------------------------------------------------------------
void
IRepProcessor::RemoveClips(IRep& irep, const vector<string>& clipNames) {
    for (auto iter = irep.AnimClips.begin(); iter != irep.AnimClips.end();) {
        if (contains(clipNames, iter->Name)) {
            irep.AnimClips.erase(iter);
        }
        else {
            iter++;
        }
    }
}
