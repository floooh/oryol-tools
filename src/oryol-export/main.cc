//------------------------------------------------------------------------------
//  oryol-export.cc
//------------------------------------------------------------------------------
#include "ExportUtil/CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include "ExportUtil/JSONSceneDumper.h"
#include "ExportUtil/JSONMeshDumper.h"
#include "ExportUtil/Config.h"
#include "ExportModel/ModelExporter.h"

using namespace OryolTools;

int
main(int argc, const char** argv) {
    // parse arguments
    CmdLineArgs args;
    args.AddBool("-help", "show help");
    args.AddString("-model", "path to input model file", "");
    args.AddString("-config", "path to TOML config file", "");
    args.AddString("-out", "path to output file (.omdl or .omsh extension)", "");
    args.AddBool("-dump-scene", "dump input scene structure as JSON to stdout");
    args.AddBool("-dump-mesh", "dump the exported mesh as JSON to stdout");
    if (!args.Parse(argc, argv)) {
        Log::Warn("Failed to parse args\n");
        return 10;
    }

    // show help?
    if (args.HasArg("-help")) {
        Log::Info("Oryol exporter tool\n");
        Log::Info("Oryol: http://www.github.com/floooh/oryol\n");
        Log::Info("Oryol Tools: http://www.github.com/floooh/oryol-tools\n\n");
        args.ShowHelp();
        return 0;
    }

    // config file is always required
    if (!args.HasArg("-config")) {
        Log::Warn("expected arg -config\n");
        return 10;
    }

    // load config file
    Config config;
    if (!config.Load(args.GetString("-config"))) {
        return 10;
    }

    // export model?
    if (args.HasArg("-model")) {
        ModelExporter modelExporter;
        std::string inPath = args.GetString("-model");
        modelExporter.SetAiProcessFlags(config.GetAiProcessFlags());
        modelExporter.SetAiProcessSortByPTypeRemoveFlags(config.GetAiProcessSortByPTypeRemoveFlags());
        modelExporter.SetVertexLayout(config.GetLayout());
        modelExporter.SetIndexSize(config.GetIndexSize());
        if (!modelExporter.ImportScene(inPath)) {
            return 10;
        }
        if (args.HasArg("-out")) {
            std::string outPath = args.GetString("-out");
            std::string outExt;
            size_t dotIndex = std::string::npos;
            if ((dotIndex = outPath.find_last_of(".")) != std::string::npos) {
                outExt = outPath.substr(dotIndex);
            }
            if (outExt == ".omsh") {
                if (!modelExporter.ExportMesh(outPath)) {
                    return 10;
                }
            }
            else if (outExt == ".omdl") {
                if (!modelExporter.ExportModel(outPath)) {
                    return 10;
                }
            }
            else {
                Log::Fatal("unknown file extension in -out path (must be .omdl or .omsh): %s\n", outPath.c_str());
            }
        }
        if (args.HasArg("-dump-scene")) {
            std::string json = JSONSceneDumper::Dump(modelExporter.GetScene(), inPath);
            Log::Info(json.c_str());
        }
        if (args.HasArg("-dump-mesh")) {
            std::string json = JSONMeshDumper::Dump(modelExporter.GetMesh());
            Log::Info(json.c_str());
        }
        return 0;
    }
    else {
        Log::Warn("Nothing to do\n");
        return 10;
    }
}