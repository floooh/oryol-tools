fips_begin_lib(ExportUtil)
    fips_files(
        Log.cc Log.h
        CmdLineArgs.cc CmdLineArgs.h
        JSONSceneDumper.cc JSONSceneDumper.h
        JSONMeshDumper.cc JSONMeshDumper.h
        Config.cc Config.h
        Vertex.h
        IndexBuffer.cc IndexBuffer.h
        VertexWriter.cc VertexWriter.h
        VertexBuffer.cc VertexBuffer.h
        Mesh.h
        PrimitiveGroup.h
        MeshSaver.h MeshSaver.cc
    )
    fips_deps(cjson)
fips_end_lib(ExportUtil)
