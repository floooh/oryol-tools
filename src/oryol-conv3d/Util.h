#pragma once
#include "ExportUtil/Log.h"

//------------------------------------------------------------------------------
template<typename TYPE> TYPE read(FILE* fp) {
    TYPE val;
    int bytesRead = fread(&val, 1, sizeof(val), fp);
    OryolTools::Log::FailIf(bytesRead != sizeof(val), "File read error.");
    return val;
};

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
