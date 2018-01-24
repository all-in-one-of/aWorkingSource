#ifndef FnGeolibOpSuite_H
#define FnGeolibOpSuite_H

#include "FnGeolibCookInterfaceSuite.h"
#include "FnGeolibSetupInterfaceSuite.h"

extern "C" {

#define FnGeolibOpSuite_version 1

/** @brief GeolibOp suite

This suite TODO
*/
struct FnGeolibOpSuite_v1
{
    void (*setup)(FnGeolibSetupInterfaceHandle interface,
        FnGeolibSetupInterfaceSuite_v1 *interfaceSuite);

    void (*cook)(FnGeolibCookInterfaceHandle interface,
        FnGeolibCookInterfaceSuite_v1 *interfaceSuite,
        uint8_t *didAbort);

    FnAttributeHandle (*describe)();

};

}
#endif /* FnGeolibOpSuite_H */
