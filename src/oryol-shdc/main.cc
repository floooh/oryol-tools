//------------------------------------------------------------------------------
//  oryol-shdc
//------------------------------------------------------------------------------
#include "ExportUtil/CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include "spirv_hlsl.hpp"
#include "spirv_msl.hpp"
#include "pystring.h"
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
const char* type_to_oryol_uniform_type(const SPIRType& type) {
    if (type.basetype == SPIRType::Float) {
        if (type.columns == 1) {
            // scalar or vec
            switch (type.vecsize) {
                case 1: return "UniformType::Float";
                case 2: return "UniformType::Vec2";
                case 3: return "UniformType::Vec3";
                case 4: return "UniformType::Vec4";
            }
        }
        else {
            // a matrix
            if ((type.vecsize == 2) && (type.columns == 2)) {
                return "UniformType::Mat2";
            }
            else if ((type.vecsize == 3) && (type.columns == 3)) {
                return "UniformType::Mat3";
            }
            else if ((type.vecsize == 4) && (type.columns == 4)) {
                return "UniformType::Mat4";
            }
        }
    }
    else if (type.basetype == SPIRType::Int) {
        return "UniformType::Int";
    }
    else if (type.basetype == SPIRType::Boolean) {
        return "UniformType::Bool";
    }
    return "UniformType::InvalidUniformType";
}

//------------------------------------------------------------------------------
void extract_resource_info(Compiler* compiler) {
    switch (compiler->get_execution_model()) {
        case ExecutionModelVertex:      Log::Info("type: vertex shader\n"); break;
        case ExecutionModelFragment:    Log::Info("type: fragment shader\n"); break;
        default: Log::Info("unsupported shader type\n"); 
    }
    ShaderResources res = compiler->get_shader_resources();
    for (const auto& ub : res.uniform_buffers) {
        Log::Info("uniform_buffer: %s\n", ub.name.c_str());
        const SPIRType& type = compiler->get_type(ub.base_type_id);
        for (int m_index = 0; m_index < int(type.member_types.size()); m_index++) {
            string m_name = compiler->get_member_name(ub.base_type_id, m_index);
            const SPIRType& m_type = compiler->get_type(type.member_types[m_index]);
            const char* m_type_str = type_to_oryol_uniform_type(m_type);
            if (m_type.array.size() > 0) {
                Log::Info("    %s %s[%d]", m_type_str, m_name.c_str(), m_type.array[0]);
            }
            else {
                Log::Info("    %s %s\n", m_type_str, m_name.c_str());
            }
        }
    }   
    for (const auto& img : res.sampled_images) {
        Log::Info("sampled_image: %s\n", img.name.c_str());
    }
    for (const auto& input : res.stage_inputs) {
        Log::Info("input: %s\n", input.name.c_str());
    } 
    for (const auto& output : res.stage_outputs) {
        Log::Info("output: %s\n", output.name.c_str());
    }
    for (const auto& storage_image : res.storage_images) {
        Log::Info("storage_image: %s\n", storage_image.name.c_str());
    }
}

//------------------------------------------------------------------------------
void to_glsl_100(const vector<uint32_t>& spirv, const string& base_path) {
    CompilerGLSL compiler(spirv);
    extract_resource_info(&compiler);
    auto opts = compiler.get_options();
    opts.version = 100;
    opts.es = true;
    compiler.set_options(opts);
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
    to_glsl_100(spirv, base_path);
    to_glsl_120(spirv, base_path);
    to_glsl_es3(spirv, base_path);
    to_glsl_330(spirv, base_path);
    to_hlsl_sm5(spirv, base_path);
    to_mlsl(spirv, base_path);

    return 0;
}
