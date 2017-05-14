#
# project: oryol-tools
#
if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

    # cpptoml requires exceptions and RTTI enabled
    set(FIPS_EXCEPTIONS ON CACHE BOOL "Enable C++ exceptions" FORCE)
    set(FIPS_RTTI ON CACHE BOOL "Enable C++ RTTI" FORCE)
    
    fips_setup()
    fips_project(oryol-tools)
endif()

include_directories(src)
fips_add_subdirectory(src)

# on Linux link everything statically to prevent DLL version hell between Linux distros
if (FIPS_LINUX)
    set_target_properties(oryol-shdc oryol-export PROPERTIES LINK_FLAGS "-static")
endif()

if (NOT FIPS_IMPORT)
    fips_finish()
endif()
