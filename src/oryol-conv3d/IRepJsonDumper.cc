//------------------------------------------------------------------------------
//  IRepJsonDumper.cc
//------------------------------------------------------------------------------
#include "IRepJsonDumper.h"
#include "cJSON.h"
#include "ExportUtil/Log.h"

//------------------------------------------------------------------------------
std::string
IRepJsonDumper::Dump(const IRep& irep) {
    cJSON* root = cJSON_CreateObject();

    // vertex components
    if (!irep.VertexComponents.empty()) {
        cJSON* comps = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "vertex_components", comps);
        for (const auto& item : irep.VertexComponents) {
            cJSON* comp = cJSON_CreateObject();
            cJSON_AddItemToArray(comps, comp);
            cJSON_AddItemToObject(comp, "attr", cJSON_CreateString(VertexAttr::ToString(item.Attr)));
            cJSON_AddItemToObject(comp, "format", cJSON_CreateString(VertexFormat::ToString(item.Format)));
            cJSON_AddItemToObject(comp, "offset", cJSON_CreateNumber(item.Offset));
        }
    }

    // materials
    if (!irep.Materials.empty()) {
        cJSON* mats = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "materials", mats);
        for (const auto& item : irep.Materials) {
            cJSON* mat = cJSON_CreateObject();
            cJSON_AddItemToArray(mats, mat);
            cJSON_AddItemToObject(mat, "name", cJSON_CreateString(item.Name.c_str()));
            cJSON_AddItemToObject(mat, "shader", cJSON_CreateString(item.Shader.c_str()));
            cJSON* vals = cJSON_CreateArray();
            cJSON_AddItemToObject(mat, "values", vals);
            for (const auto& valItem : item.Values) {
                cJSON* val = cJSON_CreateObject();
                cJSON_AddItemToArray(vals, val);
                cJSON_AddItemToObject(val, "name", cJSON_CreateString(valItem.Name.c_str()));
                cJSON_AddItemToObject(val, "type", cJSON_CreateString(IRep::PropType::ToString(valItem.Type)));
                cJSON_AddItemToObject(val, "value", cJSON_CreateFloatArray(&valItem.Value.x, 4));
            }
            cJSON* texs = cJSON_CreateArray();
            cJSON_AddItemToObject(mat, "textures", texs);
            for (const auto& texItem : item.Textures) {
                cJSON* tex = cJSON_CreateObject();
                cJSON_AddItemToArray(texs, tex);
                cJSON_AddItemToObject(tex, "name", cJSON_CreateString(texItem.Name.c_str()));
                cJSON_AddItemToObject(tex, "loc", cJSON_CreateString(texItem.Location.c_str()));
            }
        }
    }

    // meshes
    if (!irep.Meshes.empty()) {
        cJSON* meshes = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "meshes", meshes);
        for (const auto& item : irep.Meshes) {
            cJSON* mesh = cJSON_CreateObject();
            cJSON_AddItemToArray(meshes, mesh);
            cJSON_AddItemToObject(mesh, "material", cJSON_CreateNumber(item.Material));
            cJSON_AddItemToObject(mesh, "first_vertex", cJSON_CreateNumber(item.FirstVertex));
            cJSON_AddItemToObject(mesh, "num_vertices", cJSON_CreateNumber(item.NumVertices));
            cJSON_AddItemToObject(mesh, "first_index", cJSON_CreateNumber(item.FirstIndex));
            cJSON_AddItemToObject(mesh, "num_indices", cJSON_CreateNumber(item.NumIndices));
            cJSON_AddItemToObject(mesh, "size", cJSON_CreateFloatArray(&item.Size.x, 3));
        }
    }

    // bones
    if (!irep.Bones.empty()) {
        cJSON* bones = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "bones", bones);
        for (const auto& item : irep.Bones) {
            cJSON* bone = cJSON_CreateObject();
            cJSON_AddItemToArray(bones, bone);
            cJSON_AddItemToObject(bone, "name", cJSON_CreateString(item.Name.c_str()));
            cJSON_AddItemToObject(bone, "parent", cJSON_CreateNumber(item.Parent));
            cJSON_AddItemToObject(bone, "pose_translate", cJSON_CreateFloatArray(&item.Position.x, 3));
            cJSON_AddItemToObject(bone, "pose_rotate", cJSON_CreateFloatArray(&item.Rotation.x, 4));
            cJSON_AddItemToObject(bone, "pose_scale", cJSON_CreateFloatArray(&item.Scaling.x, 3));
        }
    }

    // nodes
    if (!irep.Nodes.empty()) {
        cJSON* nodes = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "nodes", nodes);
        for (const auto& item : irep.Nodes) {
            cJSON* node = cJSON_CreateObject();
            cJSON_AddItemToArray(nodes, node);
            cJSON_AddItemToObject(node, "name", cJSON_CreateString(item.Name.c_str()));
            cJSON_AddItemToObject(node, "parent", cJSON_CreateNumber(item.Parent));
            cJSON_AddItemToObject(node, "translate", cJSON_CreateFloatArray(&item.Position.x, 3));
            cJSON_AddItemToObject(node, "rotate", cJSON_CreateFloatArray(&item.Rotation.x, 4));
            cJSON_AddItemToObject(node, "scale", cJSON_CreateFloatArray(&item.Scaling.x, 3));
            if (!item.Meshes.empty()) {
                cJSON_AddItemToObject(node, "meshes", cJSON_CreateIntArray((const int*)&item.Meshes[0], item.Meshes.size()));
            }
        }
    }

    // anim key layout
    if (!irep.KeyComponents.empty()) {
        cJSON* comps = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "key_components", comps);
        for (const auto& item : irep.KeyComponents) {
            cJSON* comp = cJSON_CreateObject();
            cJSON_AddItemToArray(comps, comp);
            cJSON_AddItemToObject(comp, "type", cJSON_CreateString(IRep::KeyType::ToString(item.Type)));
        }
    }

    // anim clips
    if (!irep.AnimClips.empty()) {
        cJSON* clips = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "clips", clips);
        for (const auto& item : irep.AnimClips) {
            cJSON* clip = cJSON_CreateObject();
            cJSON_AddItemToArray(clips, clip);
            cJSON_AddItemToObject(clip, "name", cJSON_CreateString(item.Name.c_str()));
            cJSON_AddItemToObject(clip, "key_duration", cJSON_CreateNumber(item.KeyDuration));
            cJSON* curves = cJSON_CreateArray();
            cJSON_AddItemToObject(clip, "curves", curves);
            for (const auto& curveItem : item.Curves) {
                cJSON* curve = cJSON_CreateObject();
                cJSON_AddItemToArray(curves, curve);
                cJSON_AddItemToObject(curve, "key_offset", cJSON_CreateNumber(curveItem.KeyOffset));
                cJSON_AddItemToObject(curve, "static_key", cJSON_CreateFloatArray(&curveItem.StaticKey.x, 4));
            }
        }
    }

    char* rawStr = cJSON_Print(root);
    std::string jsonStr(rawStr);
    free(rawStr);
    cJSON_Delete(root);
    return jsonStr;
}
