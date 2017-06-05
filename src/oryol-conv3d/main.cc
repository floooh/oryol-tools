//------------------------------------------------------------------------------
//  oryol-conv3d/main.cc
//------------------------------------------------------------------------------ 
#include "ExportUtil/CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include "pystring.h"
#include "N3Loader.h"
#include "N3JsonDumper.h"
#include "IRepJsonDumper.h"

using namespace OryolTools;

int main(int argc, const char** argv) {
    CmdLineArgs args;
    args.AddBool("-help", "show help");
    args.AddString("-in", "input file or asset name (.n3, .fbx)", "");
    args.AddString("-out", "output filename or path", "out.orl");
    args.AddString("-indir", "optional asset root directory", "");
    args.AddString("-outdir", "optional output asset root directory", "");
    args.AddBool("-dumpin", "dump input file info to JSON");
    args.AddBool("-dumpout", "dump output file info to JSON");
    args.AddString("-n3dir", "N3 asset root directory (when loading .n3 file)", "");
    if (!args.Parse(argc, argv)) {
        Log::Fatal("Failed to parse args\n");
    }

    if (args.HasArg("-help")) {
        Log::Info("Oryol 3D asset converter tool\n");
        args.ShowHelp();
        return 0;
    }

    IRep irep;
    std::string inFile = args.GetString("-in");
    Log::FailIf(inFile.empty(), "no input file provided (-in)\n");

    // .n3 file?
    if (pystring::endswith(inFile, ".n3")) {
        std::string n3Dir = args.GetString("-n3dir");
        Log::FailIf(n3Dir.empty(), "-n3dir expected when loading .n3 file\n");
        N3Loader n3Loader;
        n3Loader.Load(inFile, n3Dir, irep);
        if (args.HasArg("-dumpin")) {
            std::string json = N3JsonDumper::Dump(n3Loader);
            Log::Info("%s\n", json.c_str());
        }
    }

    // dump intermediate representation to JSON?
    if (args.HasArg("-dumpout")) {
        std::string json = IRepJsonDumper::Dump(irep);
        Log::Info("%s\n", json.c_str());
    }
    return 0;
}
