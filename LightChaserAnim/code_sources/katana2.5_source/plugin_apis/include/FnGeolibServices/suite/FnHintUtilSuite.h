// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnHintUtilSuite_H
#define FnHintUtilSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnHintUtilHostSuite_version 1

/** @brief HintUtil host suite
*/
struct FnHintUtilHostSuite_v1
{
    FnAttributeHandle (*getHintGroup)(FnAttributeHandle inputAttr);

    FnAttributeHandle (*parseConditionalStateGrammar)(
            const char * inputExpr,
            const char * prefix,
            const char * secondaryPrefix,
            FnAttributeHandle *errorMessage);
};

}
#endif // FnHintUtilSuite_H
