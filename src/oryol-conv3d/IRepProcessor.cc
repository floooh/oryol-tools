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
void
IRepProcessor::RemoveNodes(IRep& irep, const vector<string>& nodeNames) {

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
