// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnConfigSuite_H
#define FnConfigSuite_H

extern "C" {

#define FnConfigHostSuite_version 1

struct FnConfigHostSuite_v1
{
    bool (*has)(const char *key);
    const char * (*get)(const char *key);
};

}

#endif // FnConfigSuite_H
