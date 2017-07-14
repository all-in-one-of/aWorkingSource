// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef ERRORREPORTING_H
#define ERRORREPORTING_H

#include <ProducerPacket.h>
#include <PRManPluginState.h>

#include <iostream>

namespace PRManProcedural
{
    // This function takes a std::string (produced by buffering the message
    // to report, typically by using a std::ostringstream) and outputs
    // it to std::cerr.
    //
    // This relies on statement:
    //
    // Ref (http://en.cppreference.com/w/cpp/io/cerr)
    // Unless sync_with_stdio(false) has been issued, it is safe to
    // concurrently access these objects from multiple threads for
    // both formatted and unformatted output.
    //
    // The function itself ensures the message is not intervleaved,
    // in a multithreaded environment (tested on RHEL 6 and Cent OS 5.4.).
    inline void Report_Message(const std::string &message)
    {
        std::string newMessage = message + "\n";
        std::cerr << newMessage;
        std::cerr.flush();
    }

    void Report_Error_Location(FnKat::FnScenegraphIterator sgIterator = FnKat::FnScenegraphIterator());
    void Report_Debug(const std::string& msg, PRManPluginState* sharedState, FnKat::FnScenegraphIterator sgIterator = FnKat::FnScenegraphIterator());
    void Report_Warning(const std::string& msg, FnKat::FnScenegraphIterator sgIterator = FnKat::FnScenegraphIterator());
    void Report_Error(const std::string& msg, FnKat::FnScenegraphIterator sgIterator = FnKat::FnScenegraphIterator());
    void Report_Fatal(const std::string& msg, FnKat::FnScenegraphIterator sgIterator = FnKat::FnScenegraphIterator());
    void Report_Debug(const std::string& msg, const ProducerPacket& producerPacket, PRManPluginState* sharedState);
    void Report_Debug(const std::string& riFunctionName, RtToken name, RtInt n,
                      RtToken* declarationTokens, RtPointer* values,
                      const std::string& identifier, PRManPluginState* sharedState);
    void Report_Info(const std::string& msg, FnKat::FnScenegraphIterator sgIterator = FnKat::FnScenegraphIterator());

    void Report_Debug(const std::string& msg, const std::string & identifier, PRManPluginState* sharedState);
    void Report_Warning(const std::string& msg, const std::string & identifier);
    void Report_Error(const std::string& msg, const std::string & identifier);
    void Report_Fatal(const std::string& msg, const std::string & identifier);
}

#endif

