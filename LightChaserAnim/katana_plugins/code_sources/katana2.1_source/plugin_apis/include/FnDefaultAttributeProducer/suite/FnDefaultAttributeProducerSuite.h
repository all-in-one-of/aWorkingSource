#ifndef FnDefaultAttributeProducerSuite_H
#define FnDefaultAttributeProducerSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>
#include <FnGeolib/suite/FnGeolibCookInterfaceSuite.h>

extern "C" {

#define FnDefaultAttributeProducerSuite_version 1

/** @brief DefaultAttributeProducer suite
*/
struct FnDefaultAttributeProducerSuite_v1
{
    FnAttributeHandle (*cook)(
        FnGeolibCookInterfaceHandle interface,
        FnGeolibCookInterfaceSuite_v1 *interfaceSuite,
        const char *attrRoot, int32_t stringlen1,
        const char *inputLocationPath, int32_t stringlen2,
        int32_t inputIndex,
        uint8_t * didAbort);
};

}
#endif /* FnDefaultAttributeProducerSuite_H */
