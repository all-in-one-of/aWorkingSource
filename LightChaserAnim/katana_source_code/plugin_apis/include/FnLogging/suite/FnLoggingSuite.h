// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnLoggingSuite_H
#define FnLoggingSuite_H

extern "C" {

/** @brief Defines for message severity
*/
#define kFnLoggingSeverityDebug     10
#define kFnLoggingSeverityInfo      20
#define kFnLoggingSeverityWarning   30
#define kFnLoggingSeverityError     40
#define kFnLoggingSeverityCritical  50

typedef void (*FnLogHandler)(const char *message, unsigned int severity, const char *module, void *userdata);

#define FnLoggingHostSuite_version 1

/** @brief Minimal logging module to kick off error reporting and handling in plugins
*/
struct FnLoggingHostSuite_v1
{
    // log a message
    void (*log)(const char *message, unsigned int severity, const char *module);

    // register (filtered) callbacks for immediate handling of messages
    //  (host-side key is combination of handler and userData)
    void (*registerHandler)(FnLogHandler handler, void *userData, unsigned int severityFilter,
        const char *moduleFilter);
    void (*unregisterHandler)(FnLogHandler handler, void *userData);
};

}
#endif // FnLoggingSuite_H
