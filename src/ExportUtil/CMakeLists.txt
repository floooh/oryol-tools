fips_begin_lib(ExportUtil)
    fips_files(
        Log.cc Log.h
        CmdLineArgs.cc CmdLineArgs.h
        JSONDumper.cc JSONDumper.h
        Config.cc Config.h
        Vertex.h
        IndexBuffer.h IndexBuffer.cc
        VertexWriter.h VertexWriter.cc
        VertexBuffer.h VertexBuffer.cc
    )
fips_end_lib(ExportUtil)
