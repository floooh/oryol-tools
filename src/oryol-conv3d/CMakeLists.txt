fips_begin_app(oryol-conv3d cmdline)
    fips_files(
        main.cc
        Util.h IRep.h 
        N3Loader.h N3Loader.cc
        NVX2Loader.h NVX2Loader.cc
        N3JsonDumper.h N3JsonDumper.cc
    )
    fips_deps(ExportUtil assimp pystring cjson)
fips_end_app()
