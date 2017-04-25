//------------------------------------------------------------------------------
//  oryol-shdc
//------------------------------------------------------------------------------
#include "ExportUtil/CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include "spirv_hlsl.hpp"
#include "spirv_msl.hpp"
#include "pystring.h"
#include "cJSON.h"
#include <stdio.h>

using namespace OryolTools;
using namespace spv;
using namespace spirv_cross;
using namespace std;

//------------------------------------------------------------------------------
vector<uint32_t> read_spirv_file(const string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) {
        Log::Fatal("Failed to open SPIRV file '%s'\n", path.c_str());
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp) / sizeof(uint32_t);
    fseek(fp, 0, SEEK_SET);
    vector<uint32_t> spirv(len);
    if (fread(spirv.data(), sizeof(uint32_t), len, fp) != size_t(len)) {
        Log::Fatal("Error reading SPIRV file '%s'\n", path.c_str());
    }
    fclose(fp);
    return spirv;
}

//------------------------------------------------------------------------------
void write_source_file(const string& path, const string& content) {
    FILE* fp = fopen(path.c_str(), "w");
    if (!fp) {
        Log::Fatal("Failed to open '%s' for writing\n", path.c_str());
    }
    fwrite(content.c_str(), 1, content.length(), fp);
    fclose(fp);
}

//------------------------------------------------------------------------------
const char* type_to_uniform_type(const SPIRType& type) {
    if (type.basetype == SPIRType::Float) {
        if (type.columns == 1) {
            // scalar or vec
            switch (type.vecsize) {
                case 1: return "float";
                case 2: return "vec2";
                case 3: return "vec3";
                case 4: return "vec4";
            }
        }
        else {
            // a matrix
            if ((type.vecsize == 2) && (type.columns == 2)) {
                return "mat2";
            }
            else if ((type.vecsize == 3) && (type.columns == 3)) {
                return "mat3";
            }
            else if ((type.vecsize == 4) && (type.columns == 4)) {
                return "mat4";
            }
        }
    }
    else if (type.basetype == SPIRType::Int) {
        return "int";
    }
    else if (type.basetype == SPIRType::Boolean) {
        return "bool";
    }
    Log::Fatal("Invalid member type in uniform block! (expected: float, vec2, vec3, vec4, mat2, mat3, mat4, int, bool)\n");
    return nullptr;
}

//------------------------------------------------------------------------------
const char* type_to_attr_format(const SPIRType& type) {
    if (type.basetype == SPIRType::Float && type.columns == 1) {
        switch (type.vecsize) {
            case 1: return "float";
            case 2: return "vec2";
            case 3: return "vec3";
            case 4: return "vec4";
        }
    }
    Log::Fatal("Invalid vertex attribute type! (expected: float, vec2, vec3, vec4)\n");
    return nullptr;
}

//------------------------------------------------------------------------------
cJSON* extract_resource_info(Compiler* compiler) {
    cJSON* root = cJSON_CreateObject();

    // shader stage
    const char* stage_str = nullptr;
    switch (compiler->get_execution_model()) {
        case ExecutionModelVertex: stage_str = "vs"; break;
        case ExecutionModelFragment: stage_str = "fs"; break;
        default: break;
    }
    if (stage_str) {
        cJSON_AddItemToObject(root, "stage", cJSON_CreateString(stage_str));
    }
    else {
        Log::Fatal("only vertex- or fragment-shaders allowed!\n");
    }

    // uniform blocks
    int ub_slot = 0;
    ShaderResources res = compiler->get_shader_resources();
    cJSON* ub_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "uniform_blocks", ub_array);
    for (const Resource& ub_res : res.uniform_buffers) {
        const SPIRType& ub_type = compiler->get_type(ub_res.base_type_id);
        cJSON* ub = cJSON_CreateObject();
        cJSON_AddItemToArray(ub_array, ub);
        cJSON_AddItemToObject(ub, "type", cJSON_CreateString(ub_res.name.c_str()));
        cJSON_AddItemToObject(ub, "name", cJSON_CreateString(compiler->get_name(ub_res.id).c_str()));
        cJSON_AddItemToObject(ub, "slot", cJSON_CreateNumber(ub_slot++));
        cJSON* ub_members = cJSON_CreateArray();
        cJSON_AddItemToObject(ub, "members", ub_members);
        for (int m_index = 0; m_index < int(ub_type.member_types.size()); m_index++) {
            cJSON* ub_member = cJSON_CreateObject();
            cJSON_AddItemToArray(ub_members, ub_member);
            string m_name = compiler->get_member_name(ub_res.base_type_id, m_index);
            const SPIRType& m_type = compiler->get_type(ub_type.member_types[m_index]);
            const char* m_type_str = type_to_uniform_type(m_type);
            cJSON_AddItemToObject(ub_member, "name", cJSON_CreateString(m_name.c_str()));
            cJSON_AddItemToObject(ub_member, "type", cJSON_CreateString(m_type_str));
            int num = 1;
            if (m_type.array.size() > 0) {
                num = m_type.array[0];
            }
            cJSON_AddItemToObject(ub_member, "num", cJSON_CreateNumber(num));
        }
    }

    // textures
    int tex_slot = 0;
    cJSON* tex_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "textures", tex_array);
    for (const Resource& img_res : res.sampled_images) {
        const SPIRType& img_type = compiler->get_type(img_res.type_id);
        cJSON* tex = cJSON_CreateObject();
        cJSON_AddItemToArray(tex_array, tex);
        const char* tex_type_str = nullptr;
        if (img_type.image.arrayed) {
            if (img_type.image.dim == Dim2D) {
                tex_type_str = "sampler2DArray";
            }
        }
        else {
            switch (img_type.image.dim) {
                case Dim2D:     tex_type_str = "sampler2D"; break;
                case DimCube:   tex_type_str = "samplerCube"; break;
                case Dim3D:     tex_type_str = "sampler3D"; break;
                default:        break;
            }
        }
        if (!tex_type_str) {
            Log::Fatal("Invalid texture type! (expected: 2D, Cube, 3D or 2D-array)\n");
        }
        cJSON_AddItemToObject(tex, "name", cJSON_CreateString(img_res.name.c_str()));
        cJSON_AddItemToObject(tex, "type", cJSON_CreateString(tex_type_str));
        cJSON_AddItemToObject(tex, "slot", cJSON_CreateNumber(tex_slot++));
    }

    // stage inputs
    cJSON* inputs_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "inputs", inputs_array);
    for (const Resource& stage_input : res.stage_inputs) {
        cJSON* input = cJSON_CreateObject();
        cJSON_AddItemToArray(inputs_array, input);
        const SPIRType& type = compiler->get_type(stage_input.base_type_id);
        const char* type_str = type_to_attr_format(type);
        cJSON_AddItemToObject(input, "name", cJSON_CreateString(stage_input.name.c_str()));
        cJSON_AddItemToObject(input, "type", cJSON_CreateString(type_str));
        if (compiler->get_execution_model() == ExecutionModelVertex) {
            uint32_t loc = compiler->get_decoration(stage_input.id, DecorationLocation);
            cJSON_AddItemToObject(input, "slot", cJSON_CreateNumber(loc));
        }
    }

    // stage outputs
    cJSON* outputs_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "outputs", outputs_array);
    for (const Resource& stage_output : res.stage_outputs) {
        cJSON* output = cJSON_CreateObject();
        cJSON_AddItemToArray(outputs_array, output);
        const SPIRType& type = compiler->get_type(stage_output.base_type_id);
        const char* type_str = type_to_attr_format(type);
        cJSON_AddItemToObject(output, "name", cJSON_CreateString(stage_output.name.c_str()));
        cJSON_AddItemToObject(output, "type", cJSON_CreateString(type_str));
    }

    return root;
}

//------------------------------------------------------------------------------
void fix_vertex_attr_locations(Compiler* compiler) {
    if (compiler->get_execution_model() == ExecutionModelVertex) {
        ShaderResources res = compiler->get_shader_resources();
        for (const auto& input : res.stage_inputs) {
            int loc = -1;
            if (input.name == "position")       loc = 0;
            else if (input.name == "normal")    loc = 1;
            else if (input.name == "texcoord0") loc = 2;
            else if (input.name == "texcoord1") loc = 3;
            else if (input.name == "texcoord2") loc = 4;
            else if (input.name == "texcoord3") loc = 5;
            else if (input.name == "tangent")   loc = 6;
            else if (input.name == "binormal")  loc = 7;
            else if (input.name == "weights")   loc = 8;
            else if (input.name == "indices")   loc = 9;
            else if (input.name == "color0")    loc = 10;
            else if (input.name == "color1")    loc = 11;
            else if (input.name == "instance0") loc = 12;
            else if (input.name == "instance1") loc = 13;
            else if (input.name == "instance2") loc = 14;
            else if (input.name == "instance3") loc = 15;
            if (-1 != loc) {
                compiler->set_decoration(input.id, DecorationLocation, (uint32_t)loc);
            }
        }
    }
}

//------------------------------------------------------------------------------
void to_reflection_json(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerGLSL compiler(spirv);
    fix_vertex_attr_locations(&compiler);
    cJSON* json = extract_resource_info(&compiler);
    char* json_raw_str = cJSON_Print(json);
    string json_str(json_raw_str);
    std::free(json_raw_str);
    cJSON_Delete(json);
    write_source_file(base_path + ".json", json_str);
}

//------------------------------------------------------------------------------
void to_glsl_100(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerGLSL compiler(spirv);
    auto opts = compiler.get_options();
    opts.version = 100;
    opts.es = true;
    compiler.set_options(opts);
    fix_vertex_attr_locations(&compiler);
    string src = compiler.compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL v100 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsl100.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_glsl_120(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerGLSL compiler(spirv);
    auto opts = compiler.get_options();
    opts.version = 120;
    opts.es = false;
    compiler.set_options(opts);
    fix_vertex_attr_locations(&compiler);
    string src = compiler.compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL v120 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsl120.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_glsl_es3(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerGLSL compiler(spirv);
    auto opts = compiler.get_options();
    opts.version = 300;
    opts.es = true;
    compiler.set_options(opts);
    fix_vertex_attr_locations(&compiler);
    string src = compiler.compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL es3 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsles3.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_glsl_330(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerGLSL compiler(spirv);
    auto opts = compiler.get_options();
    opts.version = 330;
    opts.es = false;
    compiler.set_options(opts);
    fix_vertex_attr_locations(&compiler);
    string src = compiler.compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL v330 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsl330.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_hlsl_sm5(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerHLSL compiler(spirv);
    auto opts = compiler.get_options();
    opts.shader_model = 50;
    compiler.set_options(opts);
    fix_vertex_attr_locations(&compiler);
    string src = compiler.compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile HLSL5 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".hlsl", src);
    }
}

//------------------------------------------------------------------------------
void to_mlsl(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerMSL compiler(spirv);
    fix_vertex_attr_locations(&compiler);
    string src = compiler.compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile MetalSL source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".metal", src);
    }
}

//------------------------------------------------------------------------------
int main(int argc, const char** argv) {
    CmdLineArgs args;
    args.AddBool("-help", "show help");
    args.AddString("-spirv", "SPIR-V input file", "");
    if (!args.Parse(argc, argv)) {
        Log::Warn("Failed to parse args!\n");
        return 10; 
    }

    if (args.HasArg("-help")) {
        Log::Info("Oryol SPIR-V to GLSL/HLSL/MSL cross-compiler\n"
                  "Based on SPIRV-Cross: https://github.com/KhronosGroup/SPIRV-Cross\n");
        args.ShowHelp();
        return 0;                  
    }
    string spirv_path = args.GetString("-spirv");
    if (spirv_path.empty()) {
        Log::Fatal("-spirv arg expected");
    }

    // load SPIRV byte code
    auto spirv = read_spirv_file(spirv_path);

    // ...translate and write to output files...
    string base_path, ext;
    pystring::os::path::splitext(base_path, ext, spirv_path);
    to_reflection_json(spirv, base_path);
    to_glsl_100(spirv, base_path);
    to_glsl_120(spirv, base_path);
    to_glsl_es3(spirv, base_path);
    to_glsl_330(spirv, base_path);
//    to_hlsl_sm5(spirv, base_path);
//    to_mlsl(spirv, base_path);

    return 0;
}
