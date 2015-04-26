#
# project: oryol-tools
#
if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

    # cpptoml requires exceptions enabled
    set(FIPS_EXCEPTIONS ON CACHE BOOL "Enable C++ exceptions" FORCE)
    
    fips_setup()
    fips_project(oryol-tools)
endif()

include_directories(src)
fips_add_subdirectory(src)

if (NOT FIPS_IMPORT)
    fips_finish()
endif()
