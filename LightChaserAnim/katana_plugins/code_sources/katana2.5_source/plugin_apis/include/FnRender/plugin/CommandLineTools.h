// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_COMMANDLINETOOLS_H
#define FNRENDER_COMMANDLINETOOLS_H

#include <stdio.h>
#include <map>
#include <string>
#include <vector>

#include "FnRender/FnRenderAPI.h"
#include "FnPlatform/internal/Portability.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

    typedef std::map<std::string, std::string>   CommandLineArguments;
    typedef std::vector<std::string>             RequiredCommandLineArguments;

    FNKAT_DEPRECATED FNRENDER_API int addArgument(
        CommandLineArguments& commandLineArguments,
        const std::string & argument, const std::string & value,
        bool allowEmptyValues = false);

    FNKAT_DEPRECATED FNRENDER_API int processCommandLineArguments(
        int argc, char* argv[], CommandLineArguments& commandLineArguments);

    FNKAT_DEPRECATED FNRENDER_API int getArgumentValue(
        const CommandLineArguments& commandLineArguments,
        const std::string & argument, std::string& value);

    FNKAT_DEPRECATED FNRENDER_API int checkRequiredArgs(
        const CommandLineArguments & commandLineArguments,
        RequiredCommandLineArguments& requiredCommandLineArguments);

    FNKAT_DEPRECATED FNRENDER_API int serialiseCommandLineArguments(
        CommandLineArguments& commandLineArguments,
        std::string& serializedCommandLine);

}
}
}

namespace FnKat = Foundry::Katana;

#endif
