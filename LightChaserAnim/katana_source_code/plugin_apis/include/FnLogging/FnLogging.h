// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnLogging_H
#define FnLogging_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnLogging/suite/FnLoggingSuite.h>

#include <string>
#include <sstream>
#include <vector>

namespace Foundry
{
  namespace Katana
  {
    namespace FnLogging
    {
        FnPlugStatus setHost(FnPluginHost *host);

        class FnLog
        {
            public:
                FnLog(const std::string &module="");
                ~FnLog();

                void log(const std::string &message, unsigned int severity) const;

                void debug(const std::string &message) const    {log(message, kFnLoggingSeverityDebug);}
                void info(const std::string &message) const     {log(message, kFnLoggingSeverityInfo);}
                void warning(const std::string &message) const  {log(message, kFnLoggingSeverityWarning);}
                void error(const std::string &message) const    {log(message, kFnLoggingSeverityError);}
                void critical(const std::string &message) const {log(message, kFnLoggingSeverityCritical);}

                static FnPlugStatus setHost(FnPluginHost *host);
                static void setSuite(FnLoggingHostSuite_v1 *suite);

            private:
                // no copy/assign
                FnLog(const FnLog& rhs);
                FnLog& operator=(const FnLog& rhs);

                std::string _module;

                static const FnLoggingHostSuite_v1 *_loggingSuite;
        };

        struct FnLogQueueEntry
        {
            std::string message;
            unsigned int severity;
            std::string module;
        };

        class FnLogQueue
        {
            public:
                FnLogQueue(unsigned int severityFilter=0, const char *moduleFilter=0x0);
                ~FnLogQueue();

                void clear();

                const std::vector<FnLogQueueEntry> &getEntries() const {return _entries;}

                std::string getEntriesAsString() const;

                static void handler(const char *message, unsigned int severity, const char *module, void *userData);

                static FnPlugStatus setHost(FnPluginHost *host);
                static void setSuite(FnLoggingHostSuite_v1 *suite);

            private:
                // no copy/assign
                FnLogQueue(const FnLogQueue& rhs);
                FnLogQueue& operator=(const FnLogQueue& rhs);

                void _appendEntry(const char *message, unsigned int severity, const char *module);

                std::vector<FnLogQueueEntry> _entries;

                static const FnLoggingHostSuite_v1 *_loggingSuite;
        };
    }
  }
}

namespace FnKat = Foundry::Katana;

// Some definitions used to set up logging in a plugin source file and
// to compose log messages

#define FnLogSetup(name) static Foundry::Katana::FnLogging::FnLog _fnLog(name);

// The following macros build up string buffers automagically behind
// the scenes, reducing the amount of effort required to use logging
// first, a generalised internal version
#define FnLogInternal(logEvent, severity) \
    do { \
        std::ostringstream _log_buf; \
        _log_buf << logEvent; \
        _fnLog.log(_log_buf.str(), severity); \
    } while(0);

// and now, wrappers for all the levels

#define FnLogDebug(logEvent) FnLogInternal(logEvent, kFnLoggingSeverityDebug)
#define FnLogInfo(logEvent) FnLogInternal(logEvent, kFnLoggingSeverityInfo)
#define FnLogWarn(logEvent) FnLogInternal(logEvent, kFnLoggingSeverityWarning)
#define FnLogError(logEvent) FnLogInternal(logEvent, kFnLoggingSeverityError)
#define FnLogCritical(logEvent) FnLogInternal(logEvent, kFnLoggingSeverityCritical)


#endif // FnLogging_H
