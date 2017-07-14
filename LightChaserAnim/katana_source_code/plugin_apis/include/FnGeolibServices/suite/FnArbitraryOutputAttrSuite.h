#ifndef FnArbitraryOutputAttrSuite_H
#define FnArbitraryOutputAttrSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>

#include <stdint.h>

extern "C" {

typedef struct ArbitraryOutputAttrStruct*
        FnArbitraryOutputAttrHandle;

#define FnArbitraryOutputAttrHostSuite_version 1

struct FnArbitraryOutputAttrHostSuite_v1
{
    FnArbitraryOutputAttrHandle (*createArbitraryOutputAttr)(
        const char *arbitraryOutputName,
        FnAttributeHandle arbitraryOutputAttr,
        const char *geometryType,
        FnAttributeHandle geometryAttr);

    void (*destroyArbitraryOutputAttr)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getNameAOA)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getScopeAOA)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getInputBaseTypeAOA)(
        FnArbitraryOutputAttrHandle handle);

    int32_t (*getElementSizeAOA)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getInputTypeAOA)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getOutputTypeAOA)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getInterpolationTypeAOA)(
        FnArbitraryOutputAttrHandle handle);

    uint8_t (*hasIndexedValueAttrAOA)(
        FnArbitraryOutputAttrHandle handle);

    FnAttributeHandle (*getIndexAttrAOA)(
        FnArbitraryOutputAttrHandle handle,
        uint8_t validateIndices);

    FnAttributeHandle (*getIndexedValueAttrAOA)(
        FnArbitraryOutputAttrHandle handle,
        const char *baseOutputType);

    FnAttributeHandle (*getValueAttrAOA)(
        FnArbitraryOutputAttrHandle handle,
        const char *baseOutputType);

    uint8_t (*isValidAOA)(
        FnArbitraryOutputAttrHandle handle);

    const char * (*getErrorAOA)(
        FnArbitraryOutputAttrHandle handle);

    uint8_t (*isArrayAOA)(
        FnArbitraryOutputAttrHandle handle);
};

}


#endif
