// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnScenegraphIteratorSuite_H
#define FnScenegraphIteratorSuite_H

extern "C" {

#include <FnAttribute/suite/FnAttributeSuite.h>

typedef struct FnSgIteratorStruct * FnSgIteratorHandle;

#define FnSgIteratorHostSuite_version 3

struct FnSgIteratorHostSuite_v3
{
    void (*retainSgIterator)(FnSgIteratorHandle handle);
    void (*releaseSgIterator)(FnSgIteratorHandle handle);

    // Loads a serialized Op tree into a Group Attribute
    FnAttributeHandle (*getOpTreeDescriptionFromFile)(
            const char* filePath);

    // Creates an iterator from a serialized Op tree
    FnSgIteratorHandle (*getIteratorFromFile)(const char *filePath);

    // Creates an iterator from a Group Attribute representing an Op tree
    FnSgIteratorHandle (*getIteratorFromOpTreeDescription)(
            FnAttributeHandle opTreeDescription);

    // Iteration functions
    FnSgIteratorHandle (*getFirstChild)(
            FnSgIteratorHandle handle, uint8_t evict);
    FnSgIteratorHandle (*getNextSibling)(
            FnSgIteratorHandle handle, uint8_t evict);
    FnSgIteratorHandle (*getParent)(FnSgIteratorHandle handle);
    FnSgIteratorHandle (*getRoot)(FnSgIteratorHandle handle);
    FnSgIteratorHandle (*getChildByName)(
            FnSgIteratorHandle handle, const char *name, uint8_t evict);
    FnSgIteratorHandle (*getByPath)(
        FnSgIteratorHandle handle, const char *path, uint8_t evict);

    // Location / Producer info
    FnAttributeHandle (*getName)(FnSgIteratorHandle handle);
    FnAttributeHandle (*getFullName)(FnSgIteratorHandle handle);

    // Attributes
    FnAttributeHandle (*getAttribute)(
            FnSgIteratorHandle handle, const char * name);
    FnAttributeHandle (*getGlobalAttribute)(
            FnSgIteratorHandle handle, const char * name);
    FnAttributeHandle (*getAttributeNames)(FnSgIteratorHandle handle);
    FnAttributeHandle (*getGlobalXFormGroup)(FnSgIteratorHandle handle);

    FnAttributeHandle (*getPotentialChildren)(FnSgIteratorHandle handle);
};


struct FnSgIteratorHostSuite_v2
{
    void (*retainSgIterator)(FnSgIteratorHandle handle);
    void (*releaseSgIterator)(FnSgIteratorHandle handle);

    // Iterators creation from a Producers tree definition (currently in Python or geolib3 op serialization)
    FnSgIteratorHandle (*getIteratorFromFile)(const char *filePath);

    // Iteration functions
    FnSgIteratorHandle (*getFirstChild)(FnSgIteratorHandle handle, uint8_t evict);
    FnSgIteratorHandle (*getNextSibling)(FnSgIteratorHandle handle, uint8_t evict);
    FnSgIteratorHandle (*getParent)(FnSgIteratorHandle handle);
    FnSgIteratorHandle (*getRoot)(FnSgIteratorHandle handle);
    FnSgIteratorHandle (*getChildByName)(FnSgIteratorHandle handle, const char *name, uint8_t evict);
    FnSgIteratorHandle (*getByPath)(FnSgIteratorHandle handle, const char *path, uint8_t evict);

    // Location / Producer info
    FnAttributeHandle (*getName)(FnSgIteratorHandle handle);
    FnAttributeHandle (*getFullName)(FnSgIteratorHandle handle);

    // Attributes
    FnAttributeHandle (*getAttribute)(FnSgIteratorHandle handle, const char * name);
    FnAttributeHandle (*getGlobalAttribute)(FnSgIteratorHandle handle, const char * name);
    FnAttributeHandle (*getAttributeNames)(FnSgIteratorHandle handle);
    FnAttributeHandle (*getGlobalXFormGroup)(FnSgIteratorHandle handle);

    FnAttributeHandle (*getPotentialChildren)(FnSgIteratorHandle handle);
};

}
#endif // FnScenegraphIteratorSuite_H
