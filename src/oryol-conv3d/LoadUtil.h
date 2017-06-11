#pragma once
#include "ExportUtil/Log.h"

using namespace OryolTools;

//------------------------------------------------------------------------------
inline const uint8_t* load_file(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    Log::FailIf(!fp, "Failed to open file '%s'\n", path.c_str());
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    const uint8_t* ptr = (const uint8_t*) malloc(size);
    int bytesRead = fread((void*)ptr, 1, size, fp);
    Log::FailIf(bytesRead != size, "Failed reading file '%s' into memory\n", path.c_str());
    fclose(fp);
    return ptr; 
}

//------------------------------------------------------------------------------
inline void free_file_data(const uint8_t* ptr) {
    free((void*)ptr);
}

//------------------------------------------------------------------------------
template<typename TYPE> TYPE read(FILE* fp) {
    TYPE val;
    int bytesRead = fread(&val, 1, sizeof(val), fp);
    OryolTools::Log::FailIf(bytesRead != sizeof(val), "File read error.");
    return val;
}

//------------------------------------------------------------------------------
template<> inline std::string read<std::string>(FILE* fp) {
    uint16_t len = read<uint16_t>(fp);
    OryolTools::Log::FailIf(len > 4096, "Long string in file.");
    std::string str;
    str.reserve(len);
    for (uint16_t i = 0; i < len; i++) {
        str.push_back(read<char>(fp));
    }
    return str;
}
