//------------------------------------------------------------------------------
//  CmdLineArgs.cc
//------------------------------------------------------------------------------
#include "CmdLineArgs.h"
#include "ExportUtil/Log.h"
#include <cassert>

namespace OryolTools {

//------------------------------------------------------------------------------
void
CmdLineArgs::AddString(const char* name, const char* desc, const char* defaultVal) {
    assert(name && desc && defaultVal);
    assert(!this->HasArg(name));

    Arg arg;
    arg.type = Arg::String;
    arg.name = name;
    arg.desc = desc;
    arg.stringValue = defaultVal;
    this->args.push_back(arg);
}

//------------------------------------------------------------------------------
void
CmdLineArgs::AddBool(const char* name, const char* desc) {
    assert(name && desc);
    assert(!this->HasArg(name));

    Arg arg;
    arg.type = Arg::Bool;
    arg.name = name;
    arg.desc = desc;
    arg.boolValue = false;
    this->args.push_back(arg);
}

//------------------------------------------------------------------------------
int
CmdLineArgs::findArg(const char* name) const {
    assert(name);
    int i = 0;
    for (const auto& arg : this->args) {
        if (arg.name == name) {
            return i;
        }
        i++;
    }
    return -1;
}

//------------------------------------------------------------------------------
bool
CmdLineArgs::HasArg(const char* name) const {
    assert(name);
    int i = this->findArg(name);
    if (-1 != i) {
        switch (this->args[i].type) {
            case Arg::String:   return !this->args[i].stringValue.empty();
            case Arg::Bool:     return this->args[i].boolValue;
            default:            return false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
std::string
CmdLineArgs::GetString(const char* name) const {
    assert(name);
    int i = this->findArg(name);
    if (i != -1) {
        if (this->args[i].type == Arg::String) {
            return this->args[i].stringValue;
        }
        else {
            Log::Fatal("CmdLineArgs::GetString(%s): arg has not been declared as string!\n", name);
        }
    }
    else {
        Log::Fatal("CmdLineArgs::GetString(%s): arg not found!\n", name);
    }
    return "";
}

//------------------------------------------------------------------------------
bool
CmdLineArgs::GetBool(const char* name) const {
    assert(name);
    int i = this->findArg(name);
    if (i != -1) {
        if (this->args[i].type == Arg::Bool) {
            return this->args[i].boolValue;
        }
        else {
            Log::Fatal("CmdLineArgs::GetBool(%s): arg has not been declared as bool!\n", name);
        }
    }
    else {
        Log::Fatal("CmdLienArgs::GetBool(%s): arg '%s' not found!\n", name);
    }
    return false;
}

//------------------------------------------------------------------------------
void
CmdLineArgs::ShowHelp() const {
    for (const auto& arg : this->args) {
        Log::Info("%s\t\t-- %s\n", arg.name.c_str(), arg.desc.c_str());
    }
}

//------------------------------------------------------------------------------
bool
CmdLineArgs::Parse(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        const char* curArgName = argv[i];
        bool foundArg = false;
        for (auto& arg : this->args) {
            if (arg.name == curArgName) {
                if (arg.type == Arg::String) {
                    if (++i < argc) {
                        arg.stringValue = argv[i];
                        if (arg.stringValue[0] == '-') {
                            Log::Warn("CmdLineArgs::Parse(): expected arg string after '%s', got '%s'\n", curArgName, argv[i]);
                            return false;
                        }
                        foundArg = true;
                        break;
                    }
                    else {
                        Log::Warn("CmdLineArgs::Parse(): expected arg string after '%s', got nothing\n", curArgName);
                        return false;
                    }
                }
                else {
                    assert(arg.type == Arg::Bool);
                    arg.boolValue = true;
                    foundArg = true;
                    break;
                }
            }
        }
        if (!foundArg) {
            Log::Warn("CmdLineArgs::Parse(): unknown arg '%s'\n", curArgName);
            return false;
        }
    }
    return true;
}

} // namespace OryolTools