// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>
#include <ri.h>
#include <rx.h>
#include <dlfcn.h>
#ifndef _WIN32
#include <libgen.h> // dirname and basenam
#endif
#include <pystring/pystring.h>
#include <exception>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstring>

#include <ErrorReporting.h>

using namespace FnKat;

namespace PRManProcedural
{
//////////////////////////////////////////////////////////////////////////////
//
// Report_XXXXXXXXX functions
//
void Report_Error_Location(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) return;

    std::ostringstream os;
    os << "[KatanaProcedural] Error: " << sgIterator.getFullName() << " - ";

    FnAttribute::StringAttribute msg = sgIterator.getAttribute("errorMessage");
    if (!msg.isValid())
    {
        os << "No errorMessage attribute found.";
    }
    else
    {
        for (unsigned long idx = 0; idx < msg.getNumberOfValues(); ++idx)
        {
            os << msg.getNearestSample(0)[idx] << "\n";
        }
    }
    Report_Message(os.str());
}

void Report_Debug(const std::string& msg, PRManPluginState* sharedState, FnScenegraphIterator sgIterator)
{
    if (!sharedState->proceduralSettings.debugStream) return;
    std::ostream* debugStream = sharedState->proceduralSettings.debugStream;

    *debugStream << "[KatanaProcedural] Debug: ";

    if (sgIterator.isValid())
    {
        *debugStream << sgIterator.getFullName() << " - ";
    }

    *debugStream << msg << std::endl;
}

void Report_Debug(const std::string& msg, const std::string & identifier, PRManPluginState* sharedState)
{
    if (!sharedState->proceduralSettings.debugStream) return;
    std::ostream* debugStream = sharedState->proceduralSettings.debugStream;

    *debugStream << "[KatanaProcedural] Debug: " << identifier << " - " << msg
                 << std::endl;
}

void Report_Debug(const std::string& riFunctionName, RtToken name, RtInt n,
                  RtToken* declarationTokens, RtPointer* values,
                  const std::string& identifier, PRManPluginState* sharedState)
{
    if (!sharedState->proceduralSettings.debugStream) return;

    std::ostringstream ss;
    ss << riFunctionName << "(\"" << name << "\", " << n << ", [\"";
    for (int i = 0; i < n; ++i)
    {
        ss << declarationTokens[i];
        if (i < n - 1)
        {
            ss << "\", \"";
        }
    }
    ss << "\"], [";
    for (int i = 0; i < n; ++i)
    {
        int numberOfValues = -1;
        std::vector<std::string> parts;
        pystring::split(declarationTokens[i], parts);
        for (size_t t = 0; t < parts.size(); ++t)
        {
            std::string part = parts[t];
            if (pystring::startswith(part, "point[")
                || pystring::startswith(part, "vector[")
                || pystring::startswith(part, "normal["))
            {
                pystring::split(part, parts, "[");
                std::string numberOfTuplesStr = parts[1];
                numberOfTuplesStr = pystring::strip(numberOfTuplesStr, "[]");
                if (pystring::isdigit(numberOfTuplesStr))
                {
                    int numberOfTuples = atoi(numberOfTuplesStr.c_str());
                    numberOfValues = numberOfTuples * 3;
                }
            }
        }
        if (numberOfValues > -1)
        {
            ss << "[";
            float* numbers = (float*) values[i];
            for (int t = 0; t < numberOfValues; ++t)
            {
                ss << numbers[t];
                if (t < numberOfValues - 1)
                {
                    ss << ", ";
                }
            }
            ss << "]";
        } else if (pystring::startswith(declarationTokens[i], "string ")
                   || strcmp(declarationTokens[i], "__handleid") == 0)
        {
            char** text = (char**) (values[i]);
            ss << "\"" << *text << "\"";
        }

        if (i < n - 1)
        {
            ss << ", ";
        }
    }
    ss << "])";
    Report_Debug(ss.str(), identifier, sharedState);
}

void Report_Debug(const std::string& msg, const ProducerPacket& producerPacket, PRManPluginState* sharedState)
{
    if (!sharedState->proceduralSettings.debugStream) return;
    std::ostream* debugStream = sharedState->proceduralSettings.debugStream;

    *debugStream << "[KatanaProcedural] Debug: ";

    if(!producerPacket.scenegraphLocation.empty())
    {
        *debugStream << producerPacket.scenegraphLocation << " - ";
    }
    else if(producerPacket.sgIterator.isValid())
    {
        *debugStream << ProducerPacket(producerPacket).sgIterator.getFullName()
                     << ": ";
    }

    *debugStream << msg << std::endl;
}

void Report_Warning(const std::string& msg, FnScenegraphIterator sgIterator)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Warning: ";

    if (sgIterator.isValid())
         os << sgIterator.getFullName() << " - ";

    os << msg;
    Report_Message(os.str());
}

void Report_Error(const std::string& msg, FnScenegraphIterator sgIterator)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Error: ";

    if (sgIterator.isValid())
        os << sgIterator.getFullName() << " - ";

    os << msg;
    Report_Message(os.str());
}

void Report_Info(const std::string& msg, FnScenegraphIterator sgIterator)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Info: ";

    if (sgIterator.isValid())
        os << sgIterator.getFullName() << " - ";

    os << msg;
    Report_Message(os.str());
}

void Report_Fatal(const std::string& msg, FnScenegraphIterator sgIterator)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Fatal Error: ";

    if (sgIterator.isValid())
        os << sgIterator.getFullName() << " - ";

    os << msg;
    Report_Message(os.str());

    throw std::runtime_error(os.str());
}

void Report_Warning(const std::string& msg, const std::string & identifier)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Warning: " << identifier << " - " << msg;

    Report_Message(os.str());
}

void Report_Error(const std::string& msg, const std::string & identifier)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Error: " << identifier << " - " << msg;

    Report_Message(os.str());
}

void Report_Fatal(const std::string& msg, const std::string & identifier)
{
    std::ostringstream os;
    os << "[KatanaProcedural] Fatal Error: " << identifier << " - " << msg;

    Report_Message(os.str());
    throw std::runtime_error(os.str());
}


} // namespace PRManProcedural
