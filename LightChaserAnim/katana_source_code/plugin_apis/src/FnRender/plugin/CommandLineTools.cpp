// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/CommandLineTools.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

int addArgument(CommandLineArguments& commandLineArguments, const std::string & argument, const std::string & value, bool allowEmptyValues)
{
    if(value == "" && !allowEmptyValues)
        return 1;

    commandLineArguments[argument] = value;

    return 0;
}

int checkRequiredArgs(const CommandLineArguments & commandLineArguments, RequiredCommandLineArguments &requiredCommandLineArguments)
{
    RequiredCommandLineArguments missingArguments;
    for (RequiredCommandLineArguments::iterator argument = requiredCommandLineArguments.begin(); argument != requiredCommandLineArguments.end(); argument++)
    {
        std::string value;
        std::string arg = *argument;
        if (!getArgumentValue(commandLineArguments, arg, value))
        {
            missingArguments.push_back(arg);
        }
    }

    if (missingArguments.size() > 0)
    {
        for (RequiredCommandLineArguments::iterator argument = missingArguments.begin(); argument != missingArguments.end(); argument++)
        {
            std::string arg = *argument;
            printf("Missing %s\n", arg.c_str());
        }
        return -1;
    }

    return 0;
}

int getArgumentValue(const CommandLineArguments& commandLineArguments, const std::string & argument, std::string &value)
{
    CommandLineArguments::const_iterator iter = commandLineArguments.find(argument);
    if (iter == commandLineArguments.end())
    {
        return 0;
    }

    value = iter->second;
    return int(value.size());
}

int processCommandLineArguments(int argc, char* argv[], CommandLineArguments& commandLineArguments)
{
    const int PROCESS_OPTION = 1;
    const int PROCESS_OPTION_ARG = 2;
    int state = PROCESS_OPTION;

    if (argc < 3)
    {
        return -1;
    }

    int currentOption = -1;
    for (int i = 0; i < argc; i++)
    {
        switch (state)
        {
            case PROCESS_OPTION:
                if (argv[i][0] == '-')
                {
                    currentOption = i;
                    state = PROCESS_OPTION_ARG;
                }
                break;
            case PROCESS_OPTION_ARG:
                commandLineArguments[&argv[currentOption][1]] =  argv[i];
                state = PROCESS_OPTION;
                break;
            default:
                break;
        }
    }

    return 0;
}

int serialiseCommandLineArguments(CommandLineArguments& commandLineArguments, std::string& serializedCommandLine)
{
    for (CommandLineArguments::iterator iter = commandLineArguments.begin(); iter != commandLineArguments.end(); iter++)
    {
        std::string argumentName = iter->first;
        std::string argumentValue = iter->second;

        serializedCommandLine += " -" + argumentName + " " + argumentValue;
    }

    return 0;
}

}
}
}
