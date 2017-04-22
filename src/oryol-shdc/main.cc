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
void to_glsl_100(const vector<uint32_t>& spirv, const string& base_path) {
    auto compiler = unique_ptr<CompilerGLSL>(new CompilerGLSL(spirv));
    auto opts = compiler->get_options();
    opts.version = 100;
    opts.es = true;
    compiler->set_options(opts);
    string src = compiler->compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL v100 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsl100.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_glsl_120(const vector<uint32_t>& spirv, const string& base_path) {
    auto compiler = unique_ptr<CompilerGLSL>(new CompilerGLSL(spirv));
    auto opts = compiler->get_options();
    opts.version = 120;
    opts.es = false;
    compiler->set_options(opts);
    string src = compiler->compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL v120 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsl120.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_glsl_es3(const vector<uint32_t>& spirv, const string& base_path) {
    auto compiler = unique_ptr<CompilerGLSL>(new CompilerGLSL(spirv));
    auto opts = compiler->get_options();
    opts.version = 300;
    opts.es = true;
    compiler->set_options(opts);
    string src = compiler->compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL es3 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsles3.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_glsl_330(const vector<uint32_t>& spirv, const string& base_path) {
    string dst_path = base_path + ".glsl330.glsl";
    auto compiler = unique_ptr<CompilerGLSL>(new CompilerGLSL(spirv));
    auto opts = compiler->get_options();
    opts.version = 330;
    opts.es = false;
    compiler->set_options(opts);
    string src = compiler->compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile GLSL v330 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".glsl330.glsl", src);
    }
}

//------------------------------------------------------------------------------
void to_hlsl_sm5(const vector<uint32_t>& spirv, const string& base_path) {
    string dst_path = base_path + ".hlsl";
    auto compiler = unique_ptr<CompilerHLSL>(new CompilerHLSL(spirv));
    auto opts = compiler->get_options();
    opts.shader_model = 50;
    compiler->set_options(opts);
    string src = compiler->compile();
    if (src.empty()) {
        Log::Fatal("Failed to compile HLSL5 source for '%s'!\n", base_path.c_str());
    }
    else {
        write_source_file(base_path + ".hlsl", src);
    }
}

//------------------------------------------------------------------------------
void to_mlsl(const vector<uint32_t>& spirv, const string& base_path) {
    string dst_path = base_path + ".metal";
    auto compiler = unique_ptr<CompilerMSL>(new CompilerMSL(spirv));
    string src = compiler->compile();
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
