#pragma once
//------------------------------------------------------------------------------
/**
    @class OryolTools::CmdLineArgs
    @brief simple command line parser
*/
#include <vector>
#include <string>

namespace OryolTools {

class CmdLineArgs {
public:
    /// add a string arg
    void AddString(const char* name, const char* desc, const char* defaultVal);
    /// add a bool arg
    void AddBool(const char* name, const char* desc);

    /// parse arguments
    bool Parse(int argc, const char** argv);
    /// display help
    void ShowHelp() const;

    /// test if arg exists
    bool HasArg(const char* name) const;
    /// get string value of arg
    std::string GetString(const char* name) const;
    /// get bool value of arg
    bool GetBool(const char* name) const;

private:
    /// get argument index, or -1 if not exists
    int findArg(const char* name) const;

    struct Arg {
        enum Type {
            String,
            Bool,
        } type;
        std::string name;
        std::string desc;
        std::string stringValue;
        bool boolValue = false;

    };
    std::string appDesc;
    std::vector<Arg> args;
};

} // namespace OryolTools
