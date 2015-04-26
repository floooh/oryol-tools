//------------------------------------------------------------------------------
//  oryol-export.cc
//------------------------------------------------------------------------------
#include "ExportUtil/CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include "ExportUtil/JSONDumper.h"
#include "ExportModel/ModelExporter.h"

using namespace OryolTools;

int
main(int argc, const char** argv) {
    // parse arguments
    CmdLineArgs args;
    args.AddBool("-help", "show help");
    args.AddString("-model", "path to input model file", "");
    args.AddString("-out", "path to output file", "");
    args.AddBool("-dump", "dump scene structure as JSON to stdout");
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

    // export model?
    if (args.HasArg("-model")) {
        ModelExporter modelExporter;
        std::string inPath = args.GetString("-model");
        if (!modelExporter.Import(inPath)) {
            return 10;
        }
        if (args.HasArg("-dump")) {
            std::string json = JSONDumper::Dump(modelExporter.GetScene(), inPath);
            Log::Info(json.c_str());
        }
        return 0;
    }
    else {
        Log::Warn("Nothing to do\n");
        return 10;
    }
}