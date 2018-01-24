// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnLogging/FnLogging.h>

namespace Foundry
{
  namespace Katana
  {
    namespace FnLogging
    {
        FnPlugStatus setHost(FnPluginHost *host)
        {
            FnPlugStatus status = FnLog::setHost(host);
            if (status != FnPluginStatusOK) return status;
            return FnLogQueue::setHost(host);
        }

        FnLog::FnLog(const std::string &module) : _module(module)
        {
        }

        FnLog::~FnLog()
        {
        }

        void FnLog::log(const std::string &message, unsigned int severity) const
        {
            if (_loggingSuite)
            {
                const char *module = 0x0;
                if (!_module.empty())
                {
                    module = _module.c_str();
                }

                _loggingSuite->log(message.c_str(), severity, module);
            }
        }

        FnPlugStatus FnLog::setHost(FnPluginHost *host)
        {
            if (host)
            {
                _loggingSuite = reinterpret_cast<const FnLoggingHostSuite_v1*>(host->getSuite("Logging", 1));
                if (_loggingSuite)
                {
                    return FnPluginStatusOK;
                }
            }
            return FnPluginStatusError;
        }
        void FnLog::setSuite(FnLoggingHostSuite_v1 *suite)
        {
            _loggingSuite = suite;
        }
        const FnLoggingHostSuite_v1 *FnLog::_loggingSuite = 0x0;

        FnLogQueue::FnLogQueue(unsigned int severityFilter, const char *moduleFilter)
        {
            if (_loggingSuite)
            {
                _loggingSuite->registerHandler(FnLogQueue::handler, this, severityFilter, moduleFilter);
            }
        }

        FnLogQueue::~FnLogQueue()
        {
            if (_loggingSuite)
            {
                _loggingSuite->unregisterHandler(FnLogQueue::handler, this);
            }
        }

        void FnLogQueue::clear()
        {
            _entries.clear();
        }

        void FnLogQueue::_appendEntry(const char *message, unsigned int severity, const char *module)
        {
            FnLogQueueEntry entry;
            entry.message = message;
            entry.severity = severity;
            entry.module = module;
            _entries.push_back(entry);
        }

        std::string FnLogQueue::getEntriesAsString() const
        {
            std::string returnValue;

            for (std::vector<FnLogQueueEntry>::const_iterator iter=_entries.begin();
                 iter!=_entries.end(); ++iter)
            {
                if (!returnValue.empty()) returnValue += "\n";
                returnValue += (*iter).message;
            }

            return returnValue;
        }

        void FnLogQueue::handler(const char *message, unsigned int severity, const char *module, void *userData)
        {
            FnLogQueue *obj = reinterpret_cast<FnLogQueue*>(userData);
            if (obj)
            {
                obj->_appendEntry(message, severity, module);
            }
        }

        FnPlugStatus FnLogQueue::setHost(FnPluginHost *host)
        {
            if (host)
            {
                _loggingSuite = reinterpret_cast<const FnLoggingHostSuite_v1*>(host->getSuite("Logging", 1));
                if (_loggingSuite)
                {
                    return FnPluginStatusOK;
                }
            }
            return FnPluginStatusError;
        }
        void FnLogQueue::setSuite(FnLoggingHostSuite_v1 *suite)
        {
            _loggingSuite = suite;
        }

        const FnLoggingHostSuite_v1 *FnLogQueue::_loggingSuite = 0x0;
    }
  }
}
