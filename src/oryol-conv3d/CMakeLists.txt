fips_begin_app(oryol-conv3d cmdline)
    fips_files(
        main.cc
        LoadUtil.h 
        IRep.h IRep.cc
        IRepProcessor.h IRepProcessor.cc
        N3Loader.h N3Loader.cc
        NVX2Loader.h NVX2Loader.cc
        NAX3Loader.h NAX3Loader.cc
        N3JsonDumper.h N3JsonDumper.cc
        IRepJsonDumper.h IRepJsonDumper.cc
        OrbSaver.h OrbSaver.cc
    )
    fips_deps(ExportUtil assimp pystring cjson)
fips_end_app()
