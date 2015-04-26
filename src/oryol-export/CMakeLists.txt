fips_begin_app(oryol-export cmdline)
    fips_files(main.cc)
    fips_deps(ExportUtil ExportModel assimp cjson)
fips_end_app()
