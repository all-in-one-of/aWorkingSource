#ifndef FnGeolibCookInterfaceSuite_H
#define FnGeolibCookInterfaceSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

/** @brief Blind declaration of an interface handle
*/
typedef struct FnGeolibCookInterfaceStruct * FnGeolibCookInterfaceHandle;

#define kFnKatGeolibDefaultInput -1
#define kFnKatGeolibNullInput -2

#define kFnKatGeolibCookInterfaceResetRootFalse 0
#define kFnKatGeolibCookInterfaceResetRootTrue 1
#define kFnKatGeolibCookInterfaceResetRootAuto 2

#define FnGeolibCookInterfaceSuite_version 1

/** @brief GeolibCookInterface suite

This suite provides the c interface for the interface passed to a
geolib3 op cook call.  Used to query the incoming scenegraph, and to modify
the output scenegraph.
*/

struct FnGeolibCookInterfaceSuite_v1
{

    const char * (*getOutputName)(FnGeolibCookInterfaceHandle handle,
        int32_t * stringlen);
    const char * (*getInputName)(FnGeolibCookInterfaceHandle handle,
        int32_t * stringlen);
    int32_t (*getOutputLocationPath)(FnGeolibCookInterfaceHandle handle,
        char *buffer, int32_t buffer_size);
    int32_t (*getInputLocationPath)(FnGeolibCookInterfaceHandle handle,
        char *buffer, int32_t buffer_size);
    int32_t (*getRelativeOutputLocationPath)(
        FnGeolibCookInterfaceHandle handle, char *buffer, int32_t buffer_size);
    int32_t (*getRelativeInputLocationPath)(
        FnGeolibCookInterfaceHandle handle, char *buffer, int32_t buffer_size);
    int32_t (*getRootLocationPath)(FnGeolibCookInterfaceHandle handle,
         char *buffer, int32_t buffer_size);
    uint8_t (*atRoot)(FnGeolibCookInterfaceHandle handle);

    int32_t (*getNumInputs)(FnGeolibCookInterfaceHandle handle);
    int32_t (*getInputIndex)(FnGeolibCookInterfaceHandle handle);

    const char * (*getOpType)(FnGeolibCookInterfaceHandle handle,
        int32_t * stringlen);
    FnAttributeHandle (*getOpArg)(FnGeolibCookInterfaceHandle handle,
        const char *name, int32_t stringlen);

    void * (*getPrivateData)(FnGeolibCookInterfaceHandle handle);

    // "get" funcs
    void (*prefetch)(FnGeolibCookInterfaceHandle handle,
        const char *inputLocationPath, int32_t stringlen,
        int32_t inputIndex);
    FnAttributeHandle (*getAttr)(FnGeolibCookInterfaceHandle handle,
        const char *attrname, int32_t stringlen1,
        const char *inputLocationPath, int32_t stringlen2,
        int32_t inputIndex,
        uint8_t *didAbort);
    uint8_t (*doesLocationExist)(FnGeolibCookInterfaceHandle handle,
        const char *inputLocationPath, int32_t stringlen,
        int32_t inputIndex,
        uint8_t *didAbort);
    FnAttributeHandle (*getPotentialChildren)(FnGeolibCookInterfaceHandle handle,
        const char *inputLocationPath, int32_t stringlen,
        int32_t inputIndex,
        uint8_t *didAbort);
    FnAttributeHandle (*getOutputAttr)(FnGeolibCookInterfaceHandle handle,
        const char *attrname, int32_t stringlen);

    // "set" funcs (never abort)
    void (*createChild)(FnGeolibCookInterfaceHandle handle,
        const char *name, int32_t stringlen1,
        const char *optype, int32_t stringlen2,
        FnAttributeHandle args,
        int32_t resetRoot,
        void *privateData, void (*deletePrivateData)(void *));

    void (*deleteSelf)(FnGeolibCookInterfaceHandle handle);

    void (*replaceChildren)(FnGeolibCookInterfaceHandle handle,
        const char *inputLocationPath, int32_t stringlen,
        int32_t inputIndex, uint8_t *didAbort);

    void (*deleteChildren)(FnGeolibCookInterfaceHandle handle);

    void (*deleteChild)(FnGeolibCookInterfaceHandle handle,
        const char *name, int32_t stringlen);

    void (*copyLocationToChild)(FnGeolibCookInterfaceHandle handle,
        const char *name, int32_t stringlen1,
        const char *inputLocationPath, int32_t stringlen2,
        int32_t inputIndex,
        const char * orderBefore, int32_t stringlen3,
        uint8_t *didAbort);

    void (*setAttr)(FnGeolibCookInterfaceHandle handle,
        const char *attrname, int32_t stringlen,
        FnAttributeHandle value,
        const uint8_t groupInherit);

    void (*deleteAttr)(FnGeolibCookInterfaceHandle handle,
        const char *attrname, int32_t stringlen);

    void (*deleteAttrs)(FnGeolibCookInterfaceHandle handle);

    void (*replaceAttrs)(FnGeolibCookInterfaceHandle handle,
        const char *inputLocationPath, int32_t stringlen,
        int32_t inputIndex,
        uint8_t *didAbort);

    void (*stopChildTraversal)(FnGeolibCookInterfaceHandle handle);

    void (*replaceChildTraversalOp)(FnGeolibCookInterfaceHandle handle,
        const char *optype, int32_t stringlen,
        FnAttributeHandle args,
        void *privateData, void (*deletePrivateData)(void *));

    void (*copyAttr)(FnGeolibCookInterfaceHandle handle,
        const char *dstattrname, int32_t stringlen1,
        const char *srcattrname, int32_t stringlen2,
        const uint8_t groupInherit,
        const char *inputLocationPath, int32_t stringlen3,
        int32_t inputIndex,
        uint8_t *didAbort);

    void (*extendAttr)(FnGeolibCookInterfaceHandle handle,
        const char *dstattrname, int32_t stringlen1,
        FnAttributeHandle value,
        const char *srcattrname, int32_t stringlen2,
        const uint8_t groupInherit,
        const char *inputLocationPath, int32_t stringlen3,
        int32_t inputIndex,
        uint8_t *didAbort);

    // execOp can abort if code called in the executed op aborts
    void (*execOp)(FnGeolibCookInterfaceHandle handle,
        const char *optype, int32_t stringlen,
        FnAttributeHandle args,
        uint8_t *didAbort);

    void (*resetRoot)(FnGeolibCookInterfaceHandle handle);

};

}
#endif /* FnGeolibCookInterfaceSuite_H */
