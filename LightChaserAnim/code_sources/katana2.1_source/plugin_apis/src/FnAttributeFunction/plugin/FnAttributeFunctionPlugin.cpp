// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginManager/FnPluginManager.h>

#include <FnAttributeFunction/plugin/FnAttributeFunctionPlugin.h>

#define ATTRIBUTE_FUNCTION_DEBUG_HOOK \
    extern "C" __attribute__((visibility("default"))) __attribute__((noinline))

// A no-op function to facilitate debugging. It's called when an
// AttributeFunction throws an exception.
ATTRIBUTE_FUNCTION_DEBUG_HOOK
void _fnAttributeFunctionThrow()
{
    // Prevent the function being optimised out. See
    // https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Function-Attributes.html
    __asm__("");
}

namespace
{
    void WriteIso8601Timestamp(const struct tm* components, FILE* fp)
    {
        fprintf(fp, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                components->tm_year + 1900, components->tm_mon + 1,
                components->tm_mday, components->tm_hour, components->tm_min,
                components->tm_sec);
    }

    void LogExceptionMessage(const char* message)
    {
        time_t currentTime = time(NULL);
        struct tm components;
        gmtime_r(&currentTime, &components);

        fprintf(stderr, "[");
        WriteIso8601Timestamp(&components, stderr);
        fprintf(
            stderr,
            "] (pid: %d) An Attribute Function threw an exception. Break on "
            "_fnAttributeFunctionThrow() to debug.",
            getpid());

        if (message && strlen(message))
            fprintf(stderr, " Exception text: %s", message);

        fprintf(stderr, "\n");
    }
}

FNATTRIBUTEFUNCTION_NAMESPACE_ENTER
{
    FnPlugStatus AttributeFunction::setHost(FnPluginHost* host)
    {
        _host = host;

        FnPlugStatus status;

        status = FnPluginManager::PluginManager::setHost(host);
        if (status != FnPluginStatusOK) return status;

        status = FnAttribute::Attribute::setHost(host);
        if (status != FnPluginStatusOK) return status;

        status = FnAttribute::GroupBuilder::setHost(host);
        if (status != FnPluginStatusOK) return status;

        return FnPluginStatusOK;
    }

    FnPluginHost *AttributeFunction::getHost() {return _host;}

    void AttributeFunction::flush()
    {
    }

    FnAttributeFunctionSuite_v1 AttributeFunction::createSuite(
            FnAttributeHandle (*run)(
                     FnAttributeHandle args))
    {
        FnAttributeFunctionSuite_v1 suite = {
            run
        };

        return suite;
    }

    FnAttribute::Attribute AttributeFunction::callRun(
        RunFunc runFunc, FnAttribute::Attribute args)
    {
        try
        {
            return runFunc(args);
        }
        catch (const std::exception& e)
        {
            LogExceptionMessage(e.what());
            _fnAttributeFunctionThrow();
        }
        catch (...)
        {
            LogExceptionMessage("");
            _fnAttributeFunctionThrow();
        }
        return FnAttribute::Attribute();
    }

    unsigned int AttributeFunction::_apiVersion = 1;
    const char* AttributeFunction::_apiName = "AttributeFunction";
    FnPluginHost *AttributeFunction::_host = 0x0;
}
FNATTRIBUTEFUNCTION_NAMESPACE_EXIT
