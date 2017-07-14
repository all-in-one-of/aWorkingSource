#ifndef FnAttributeFunctionSuite_H
#define FnAttributeFunctionSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnAttributeFunctionSuite_version 1

/** @brief AttributeFunction suite
*/
struct FnAttributeFunctionSuite_v1
{
    FnAttributeHandle (*run)(FnAttributeHandle args);
};

}
#endif /* FnAttributeFunctionSuite_H */
