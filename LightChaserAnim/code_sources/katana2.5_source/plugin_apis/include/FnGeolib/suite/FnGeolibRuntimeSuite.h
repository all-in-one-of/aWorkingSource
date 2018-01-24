#ifndef FnGeolibRuntimeSuite_H
#define FnGeolibRuntimeSuite_H

#include <FnGeolib/suite/FnGeolibOpSuite.h>

extern "C" {

/** @brief Blind declaration of runtime handle
*/
typedef struct FnGeolibRuntimeStruct * FnGeolibRuntimeHandle;

/** @brief Blind declaration of transaction handle
*/
typedef struct FnGeolibTransactionStruct * FnGeolibTransactionHandle;

/** @brief Id for referring to runtime op instance.

An op id can stored in an attribute (see getOpInputs).
*/
typedef int32_t FnGeolibOpId;

#define kFnKatGeolibNullOpId 0

/** @brief Id for referring to runtime client instance.
*/
typedef int32_t FnGeolibClientId;

#define kFnKatGeolibNoOp "no-op"

typedef int64_t (*FnGeolibWriteToStream)(void *stream, const void *buf, int64_t size);
typedef int64_t (*FnGeolibReadFromStream)(void *stream, void *buf, int64_t size);

#define FnGeolibRuntimeSuite_version 1

/** @brief Runtime suite

This suite provides TODO
*/
struct FnGeolibRuntimeSuite_v1
{
    FnGeolibRuntimeHandle (*createRuntime)();
    void (*destroyRuntime)(FnGeolibRuntimeHandle handle);
    void (*registerOp)(FnGeolibRuntimeHandle handle, const char *opType,
        FnGeolibOpSuite_v1 *opSuite);

    FnGeolibTransactionHandle (*createTransaction)(
        FnGeolibRuntimeHandle handle);
    void (*destroyTransaction)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn);

    FnGeolibOpId (*createOp)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn);
    void (*retainOp)(FnGeolibRuntimeHandle handle, FnGeolibOpId op);
    void (*releaseOp)(FnGeolibRuntimeHandle handle, FnGeolibOpId op);
    void (*setOpArgs)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn,
        FnGeolibOpId op, const char *opType, FnAttributeHandle args);
    void (*setOpInputs)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn, FnGeolibOpId op,
        FnGeolibOpId *inputs, int32_t numInputs);

    void (*getOpArgs)(FnGeolibRuntimeHandle handle, FnGeolibOpId op,
        const char **opType, FnAttributeHandle *args);

    // This returns an int attr, which are PRE-INCREMENTED FnGeolibClientId(s)
    // The caller MUST release all returned clients.
    FnAttributeHandle (*getOpInputs)(FnGeolibRuntimeHandle handle,
        FnGeolibOpId op);

    FnGeolibClientId (*createClient)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn);
    void (*retainClient)(FnGeolibRuntimeHandle handle, FnGeolibClientId client);
    void (*releaseClient)(FnGeolibRuntimeHandle handle, FnGeolibClientId client);
    void (*setClientOp)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn,
        FnGeolibClientId client, FnGeolibOpId op);

    /* returns a new reference to the Op, which you are reponsible to release */
    FnGeolibOpId (*getClientOp)(FnGeolibRuntimeHandle handle, FnGeolibClientId client);

    void (*commit)(FnGeolibRuntimeHandle handle, FnGeolibTransactionHandle *transactions,
        int32_t numTransactions);

    void (*setLocationsOpen)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, FnAttributeHandle locationPaths);
    void (*setLocationsClosed)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, FnAttributeHandle locationPaths);
    void (*setLocationsOpenRecursive)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, FnAttributeHandle locationPaths,
        FnAttributeHandle stopTypes);
    void (*setLocationsClosedRecursive)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, FnAttributeHandle locationPaths);
    void (*interruptOpenRecursive)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client);
    void (*setLocationsActive)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, FnAttributeHandle locationPaths);
    void (*setLocationsInactive)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, FnAttributeHandle locationPaths);
    FnAttributeHandle (*getOpenLocations)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, const char *rootLocationPath);
    FnAttributeHandle (*getActiveLocations)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, const char *rootLocationPath);
    uint8_t (*isLocationOpen)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, const char *locationPath);
    uint8_t (*isLocationActive)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, const char *locationPath);

    FnAttributeHandle (*getLocationEvents)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, int32_t maxEvents);
    void (*setEventCachingEnabled)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, uint8_t enabled);
    uint8_t (*isEventCachingEnabled)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client);
    FnAttributeHandle (*getCachedLocation)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, const char *locationPath);

    const char *(*getRootLocationPath)(FnGeolibRuntimeHandle handle);

    // location data
    FnAttributeHandle (*cookLocation)(FnGeolibRuntimeHandle handle,
        FnGeolibClientId client, const char *locationPath);

    FnAttributeHandle (*getOptions)(FnGeolibRuntimeHandle handle);

    void (*setOptions)(FnGeolibRuntimeHandle handle,
        FnAttributeHandle options);


    // op tree serialization
    uint8_t (*serializeOps)(FnGeolibRuntimeHandle handle,
        FnGeolibOpId op,
        void *writeStream, FnGeolibWriteToStream writeFunc,
        FnAttributeHandle *errorMessage);

    /** On success, returns a new reference to the root op, which you are
         reponsible to release.  Ops are not available for use until txn
         is committed.
        On failure, returns 0 (NoOpId) */
    FnGeolibOpId (*deserializeOps)(FnGeolibRuntimeHandle handle,
        FnGeolibTransactionHandle txn,
        void *readStream, FnGeolibReadFromStream readFunc,
        FnAttributeHandle *errorMessage);

    uint8_t (*isProcessing)(FnGeolibRuntimeHandle handle);

    void (*flushCaches)(FnGeolibRuntimeHandle handle);

    void (*evict)(FnGeolibRuntimeHandle handle, const char *primaryPathToKeep);

    FnAttributeHandle (*describeOp)(FnGeolibRuntimeHandle handle,
        const char* opType);

    FnAttributeHandle (*getRegisteredOpTypes)(FnGeolibRuntimeHandle handle);
};

}
#endif /* FnGeolibRuntimeSuite_H */
