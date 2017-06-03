//------------------------------------------------------------------------------
//  N3JsonDumper.cc
//------------------------------------------------------------------------------
#include "N3JsonDumper.h"
#include "cJSON.h"
#include "ExportUtil/Log.h"

//------------------------------------------------------------------------------
static const char*
nodeType(const N3Loader::N3Node& n3) {
    switch (n3.ClassTag) {
        case 'TRFN': return "TransformNode";
        case 'SPND': return "ShapeNode";
        case 'MANI': return "AnimatorNode";
        case 'PSND': return "ParticleSystemNode";
        case 'CHRN': return "CharacterNode";
        case 'CHSN': return "CharacterSkinNode";
        default: return "UNKNOWN";
    }
}

//------------------------------------------------------------------------------
static void
recurseDumpNode(cJSON* parentChildArray, const N3Loader& n3Loader, int nodeIndex) {
    cJSON* node = cJSON_CreateObject();
    cJSON_AddItemToArray(parentChildArray, node);
    const auto& n3 = n3Loader.Nodes[nodeIndex];

    cJSON_AddItemToObject(node, "name", cJSON_CreateString(n3.Name.c_str()));
    cJSON_AddItemToObject(node, "type", cJSON_CreateString(nodeType(n3)));
    cJSON_AddItemToObject(node, "parent_index", cJSON_CreateNumber(n3.Parent));
    cJSON_AddItemToObject(node, "center", cJSON_CreateFloatArray(&n3.Center.x, 4));
    cJSON_AddItemToObject(node, "extents", cJSON_CreateFloatArray(&n3.Extents.x, 4));
    cJSON_AddItemToObject(node, "position", cJSON_CreateFloatArray(&n3.Position.x, 4));
    cJSON_AddItemToObject(node, "rotation", cJSON_CreateFloatArray(&n3.Rotation.x, 4));
    cJSON_AddItemToObject(node, "scaling", cJSON_CreateFloatArray(&n3.Scaling.x, 4));
    cJSON_AddItemToObject(node, "rotate_pivot", cJSON_CreateFloatArray(&n3.RotatePivot.x, 4));
    cJSON_AddItemToObject(node, "scale_pivot", cJSON_CreateFloatArray(&n3.ScalePivot.x, 4));
    if (!n3.Shader.empty()) {
        cJSON_AddItemToObject(node, "shader", cJSON_CreateString(n3.Shader.c_str()));
    }
    if (0 < (n3.Textures.size() + n3.IntParams.size() + n3.FloatParams.size() + n3.VecParams.size())) {
        cJSON* params = cJSON_CreateArray();
        cJSON_AddItemToObject(node, "params", params);
        for (const auto& item : n3.Textures) {
            cJSON* param = cJSON_CreateObject();
            cJSON_AddItemToArray(params, param);
            cJSON_AddItemToObject(param, "name", cJSON_CreateString(item.first.c_str()));
            cJSON_AddItemToObject(param, "type", cJSON_CreateString("Texture"));
            cJSON_AddItemToObject(param, "value", cJSON_CreateString(item.second.c_str()));
        }
        for (const auto& item : n3.IntParams) {
            cJSON* param = cJSON_CreateObject();
            cJSON_AddItemToArray(params, param);
            cJSON_AddItemToObject(param, "name", cJSON_CreateString(item.first.c_str()));
            cJSON_AddItemToObject(param, "type", cJSON_CreateString("Int"));
            cJSON_AddItemToObject(param, "value", cJSON_CreateNumber(item.second));
        }
        for (const auto& item : n3.FloatParams) {
            cJSON* param = cJSON_CreateObject();
            cJSON_AddItemToArray(params, param);
            cJSON_AddItemToObject(param, "name", cJSON_CreateString(item.first.c_str()));
            cJSON_AddItemToObject(param, "type", cJSON_CreateString("Float"));
            cJSON_AddItemToObject(param, "value", cJSON_CreateNumber(item.second));
        }
        for (const auto& item : n3.VecParams) {
            cJSON* param = cJSON_CreateObject();
            cJSON_AddItemToArray(params, param);
            cJSON_AddItemToObject(param, "name", cJSON_CreateString(item.first.c_str()));
            cJSON_AddItemToObject(param, "type", cJSON_CreateString("Float"));
            cJSON_AddItemToObject(param, "value", cJSON_CreateFloatArray(&item.second.x, 4));
        }
    }
    if (!n3.Mesh.empty()) {
        cJSON_AddItemToObject(node, "mesh", cJSON_CreateString(n3.Mesh.c_str()));
        cJSON_AddItemToObject(node, "prim_group", cJSON_CreateNumber(n3.PrimGroup));
    }
    if (!n3.Animation.empty()) {
        cJSON_AddItemToObject(node, "animation", cJSON_CreateString(n3.Animation.c_str()));
    }
    if (!n3.Joints.empty()) {
        cJSON* joints = cJSON_CreateArray();
        cJSON_AddItemToObject(node, "joints", joints);
        for (const auto& item : n3.Joints) {
            cJSON* joint = cJSON_CreateObject();
            cJSON_AddItemToArray(joints, joint);
            cJSON_AddItemToObject(joint, "name", cJSON_CreateString(item.Name.c_str()));
            cJSON_AddItemToObject(joint, "parent_index", cJSON_CreateNumber(item.Parent));
            cJSON_AddItemToObject(joint, "pose_translate", cJSON_CreateFloatArray(&item.PoseTranslation.x, 4));
            cJSON_AddItemToObject(joint, "pose_rotate", cJSON_CreateFloatArray(&item.PoseRotation.x, 4));
            cJSON_AddItemToObject(joint, "pose_scale", cJSON_CreateFloatArray(&item.PoseScale.x, 4));
        }
    }
    if (!n3.SkinFragments.empty()) {
        cJSON* frags = cJSON_CreateArray();
        cJSON_AddItemToObject(node, "skin_fragments", frags);
        for (const auto& item : n3.SkinFragments) {
            cJSON* frag = cJSON_CreateObject();
            cJSON_AddItemToArray(frags, frag);
            cJSON_AddItemToObject(frag, "prim_group", cJSON_CreateNumber(item.PrimGroup));
            cJSON_AddItemToObject(frag, "joint_palette", cJSON_CreateIntArray(&item.JointPalette[0], item.JointPalette.size()));
        }
    }
    if (!n3.Children.empty()) {
        cJSON* nodes = cJSON_CreateArray();
        cJSON_AddItemToObject(node, "nodes", nodes);
        for (int childIndex : n3.Children) {
            recurseDumpNode(nodes, n3Loader, childIndex);
        }
    }
}

//------------------------------------------------------------------------------
std::string
N3JsonDumper::Dump(const N3Loader& n3) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString(n3.ModelName.c_str()));
    if (!n3.Nodes.empty()) {
        cJSON* nodes = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "nodes", nodes);
        recurseDumpNode(nodes, n3, 0);
    }
    char* rawStr = cJSON_Print(root);
    std::string jsonStr(rawStr);
    free(rawStr);
    cJSON_Delete(root);
    return jsonStr;
}
