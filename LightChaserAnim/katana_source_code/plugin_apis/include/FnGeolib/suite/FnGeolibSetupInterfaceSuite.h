#ifndef FnGeolibSetupInterfaceSuite_H
#define FnGeolibSetupInterfaceSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

/** @brief Blind declaration of an interface handle
*/

typedef struct FnGeolibSetupInterfaceStruct * FnGeolibSetupInterfaceHandle;

#define kFnKatGeolibThreadModeConcurrent 0
#define kFnKatGeolibThreadModeGlobalUnsafe 1

#define FnGeolibSetupInterfaceSuite_version 1

/** @brief GeolibSetupInterface suite

This suite provides the function interface for ops to user during the setup
phase, pre-cook.
*/
struct FnGeolibSetupInterfaceSuite_v1
{
    void (*setThreading)(FnGeolibSetupInterfaceHandle handle,
        int32_t threadMode);
};

}
#endif /* FnGeolibSetupInterfaceSuite_H */
