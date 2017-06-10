//------------------------------------------------------------------------------
//  oryol-conv3d/main.cc
//------------------------------------------------------------------------------ 
#include "ExportUtil/CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include "pystring.h"
#include "N3Loader.h"
#include "N3JsonDumper.h"
#include "IRepJsonDumper.h"
#include "OrbSaver.h"

using namespace OryolTools;

int main(int argc, const char** argv) {
    CmdLineArgs args;
    args.AddBool("-help", "show help");
    args.AddString("-in", "input file or asset name (.n3, .fbx)", "");
    args.AddString("-out", "output filename or path", "out.orb");
    args.AddString("-indir", "optional asset root directory", "");
    args.AddString("-outdir", "optional output asset root directory", "");
    args.AddBool("-dumpin", "dump input file info to JSON");
    args.AddBool("-dumpout", "dump output file info to JSON");
    args.AddBool("-dumpvtx", "dump intermediate representation vertex data");
    args.AddBool("-dumpidx", "dump intermediate representation index data");
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

    // save intermediate representation to output file
    OrbSaver orbSaver;
    orbSaver.Layout.Components.push_back(VertexComponent(VertexAttr::Position, VertexFormat::Float3));
    orbSaver.Layout.Components.push_back(VertexComponent(VertexAttr::Normal, VertexFormat::Byte4N));
    orbSaver.Layout.Components.push_back(VertexComponent(VertexAttr::TexCoord0, VertexFormat::Float2));
    orbSaver.Save(args.GetString("-out"), irep);

    // dump intermediate representation
    if (args.HasArg("-dumpout")) {
        std::string json = IRepJsonDumper::Dump(irep);
        Log::Info("%s\n", json.c_str());
    }
    if (args.HasArg("-dumpvtx")) {
        int stride = 0;
        for (const auto& comp : irep.VertexComponents) {
            stride += VertexFormat::ByteSize(comp.Format) / sizeof(float);
        }
        Log::FailIf((irep.VertexData.size() % stride) != 0, "Vertex data size isn't multiple of vertex stride!\n");
        const int numVertices = irep.VertexData.size() / stride;
        for (int i = 0; i < numVertices; i++) {
            Log::Info("%d: ", i);
            for (int j = 0; j < stride; j++) {
                Log::Info("%.4f ", irep.VertexData[i * stride + j]);
            }
            Log::Info("\n");
        }
        Log::Info("\n");
    }
    if (args.HasArg("-dumpidx")) {
        Log::FailIf((irep.IndexData.size() % 3) != 0, "Index data size isn't multiple of 3!\n");
        const int numTris = irep.IndexData.size() / 3;
        for (int i = 0; i < numTris; i++) {
            Log::Info("%d: %d %d %d\n", i, irep.IndexData[i*3+0], irep.IndexData[i*3+1], irep.IndexData[i*3+2]);
        }
    }

    return 0;
}
