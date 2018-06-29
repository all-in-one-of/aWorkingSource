// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnAssetSuite_H
#define FnAssetSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>
#include <cstddef>

extern "C" {

///@cond FN_INTERNAL_DEV

/** @defgroup FnAsset Asset API
{
 */

/** @brief Blind declaration of a file sequence handle
*/
typedef struct FnFileSequenceStruct * FnFileSequenceHandle;

/** @brief Blind declaration of a file sequence array handle
*/
typedef struct FnFileSequenceArrayStruct * FnFileSequenceArrayHandle;


#define FnFileSequencePluginSuite_version 1

/** @brief File sequence plugin suite
 *
 * This suite provides functions a file sequence plugin must provide for
 * creating and querying file sequence objects.
 *
 * File sequences are closely related to the asset system.  A file sequence spec is a
 * file path that contains tokens to be substituted with the current frame number.  Some
 * file sequence specs may also specify the frame range of the sequence.
 *
 * An example from the FileSeq plugin: /path/to/image.1-10#.exr.
 *
 * This example references a sequence of images, frames 1 through 10, with the frame
 * number zero-padded to length 4.
 *
 * Resolving this sequence at frame 5 would result in: /path/to/image.0005.exr.
 */
struct FnFileSequencePluginSuite_v1
{
    /** @brief Create a file sequence object instance for a given file sequence string.
     * @param fileseq String representing file sequence (e.g. "image.#.exr" or "image.%04d.exr").
     *
     * @return New handle (caller owns handle) to file sequence object.  If input fileseq is not a valid sequence, return 0x0 (and log error).
     */
    FnFileSequenceHandle (*create)(const char *fileseq, FnAttributeHandle *errorMessage);

    /** @brief Destroy a file sequence object instance, releasing its memory.
     * Called once per handle.  Calling on a null (0x0) handle should have no effect.
     * @param handle A valid file sequence handle created via #create.
     */
    void (*destroy)(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage);

    /** @brief Static function (requires no instance) that tests whether a give string is a valid file sequence.
     * @param fileseq String to check.
     *
     * @return True if fileseq is a valid file sequence.
     */
    bool (*isFileSequence)(const char *fileseq, FnAttributeHandle *errorMessage);

    /** @brief Builds a string that represents a valid file sequence
     * @param prefix the portion of the file sequence string that will come before the frame representation (ex. in "/a/b/c/d.#.exr" the prefix would be /a/b/c/d)
     * @param suffix the portion of the file sequence string that will come after the frame representation (ex. in "/a/b/c/d.#.exr" the suffix would be .exr)
     * @param padding the padding of the frame number - might not be needed in some types of file sequence
     *
     * @return New handle (caller owns reference) to StringAttribute with the valid file sequence.
     */
    FnAttributeHandle (*buildFileSequenceString)(const char *prefix,
                                                 const char *suffix,
                                                 int padding,
                                                 FnAttributeHandle *errorMessage);

    /** @brief Get string representation of the file sequence
     * @param handle A valid file sequence handle.
     *
     * @return New handle (caller owns reference) to StringAttribute with file sequence as string.
     */
    FnAttributeHandle (*getAsString)(FnFileSequenceHandle handle,
                                     FnAttributeHandle *errorMessage);

    /** @brief Get the directory portion of file sequence
     * @param handle A valid file sequence handle.
     *
     * @return New handle (caller owns reference) to StringAttribute with directory portion of file sequence (e.g. "/a/b/c" for fileseq "/a/b/c/d.#.exr").
     */
    FnAttributeHandle (*getDirectory)(FnFileSequenceHandle handle,
                                      FnAttributeHandle *errorMessage);

    /** @brief Get the portion of file sequence before the frame number.
     * @param handle A valid file sequence handle.
     *
     * @return New handle (caller owns reference) to StringAttribute with prefix portion of file sequence (e.g. "/a/b/c/d." for fileseq "/a/b/c/d.#.exr").
     */
    FnAttributeHandle (*getPrefix)(FnFileSequenceHandle handle,
                                   FnAttributeHandle *errorMessage);

    /** @brief Get the portion of file sequence after the frame number.
     * @param handle A valid file sequence handle.
     *
     * @return  New handle (caller owns reference) to StringAttribute with suffix portion of file sequence (e.g. ".exr" for fileseq "/a/b/c/d.#.exr").
     */
    FnAttributeHandle (*getSuffix)(FnFileSequenceHandle handle,
                                   FnAttributeHandle *errorMessage);

    /** @brief Get the padding of file sequence the frame number.
     * @param handle A valid file sequence handle.
     *
     * @return Padding amount of file sequence (e.g. 4 for fileseq that expands to image.0001.exr)
     */
    unsigned int (*getPadding)(FnFileSequenceHandle handle,
                               FnAttributeHandle *errorMessage);

    /** @brief Get the filename for file sequence with sequence portion replaced by correctly padded frame.
     * @param handle A valid file sequence handle.
     * @param frame The frame to get the filename.
     *
     * @return New handle (caller owns reference) to StringAttribute with resolved filename.  Return 0x0 (and log error) if frame is outside sequence frame range.
     */
    FnAttributeHandle (*getResolvedPath)(FnFileSequenceHandle handle, int frame,
                                         FnAttributeHandle *errorMessage);

    /** @brief Return whether file sequence has explicit frame set.
     * @param handle A valid file sequence handle.
     *
     * @return True if file sequence has explicit frame range (set) embedded in it (e.g. "image.1-10#.exr").
     */
    bool (*hasFrameSet)(FnFileSequenceHandle handle,
                        FnAttributeHandle *errorMessage);

    /** @brief Test whether frame is within explicit frame set.
     * @param handle A valid file sequence handle.
     *
     * @return True if frame is in frame set.  If file sequence does not have explicit frame set, always return true.
     */
    bool (*isFrameInFrameSet)(FnFileSequenceHandle handle, int frame,
                              FnAttributeHandle *errorMessage);

    /** @brief Return first frame in explicit frame set.
     * @param handle A valid file sequence handle.
     *
     * @return First (minimum) frame in frame set.  If file sequence does not have explicit frame set, always return 0.
     */
    int (*getFirstFrameInFrameSet)(FnFileSequenceHandle handle,
                                   FnAttributeHandle *errorMessage);

    /** @brief Return last frame in explicit frame set.
     * @param handle A valid file sequence handle.
     *
     * @return Last (maximum) frame in frame set.  If file sequence does not have explicit frame set, always return 0.
     */
    int (*getLastFrameInFrameSet)(FnFileSequenceHandle handle,
                                  FnAttributeHandle *errorMessage);

    void (*getNearestFramesInFrameSet)(FnFileSequenceHandle handle, int frame,
        bool *hasLeft, int *nearestLeft, bool *hasRight, int *nearestRight,
        FnAttributeHandle *errorMessage);

    /** @brief Get frame set as list of integers.
     * @param handle A valid file sequence handle.
     *
     * @return New handle (caller owns reference) to FnIntAttribute containing list of integer frame numbers (e.g. [1,2,3,4,5] for "image.1-5#.exr").
     */
    FnAttributeHandle (*getFrameListFromFrameSet)(FnFileSequenceHandle handle,
                                                  FnAttributeHandle *errorMessage);

    /** @brief Given a list of filenames, create a new file sequence array object containing a list of sequences and files not belonging to any sequence.
     *
     * Used by directory views (e.g. file open dialog) to group files as file sequences.
     *
     * @param fileList Array of filenames.
     * @param fileCount Length of fileList array.

     * @return New handle (caller owns handle) to file sequence array object instance.
     */
    FnFileSequenceArrayHandle (*findSequence)(const char **fileList,
                                              unsigned int fileCount,
                                              FnAttributeHandle *errorMessage);

    /** @brief Destroy a file sequence array object instance, releasing its memory.
     * Called once per handle.  Calling on a null (0x0) handle should have no effect.
     * @param handle A valid file sequence handle created via #findSequence.
     */
    void (*destroyFileSequenceArray)(FnFileSequenceArrayHandle handle,
                                     FnAttributeHandle *errorMessage);

    /** @brief Get number of file sequences in file sequence array object.
     * @param handle A valid file sequence array handle.
     *
     * @return Number of file sequences.
     */
    unsigned int (*getFileSequenceCount)(FnFileSequenceArrayHandle handle,
                                         FnAttributeHandle *errorMessage);

    /** @brief Get file sequence at index.
     * @param handle A valid file sequence array handle.
     * @param index Index of file sequence object to get.
     *
     * @return New handle (caller owns handlw) to file sequence object at index.  Return 0x0 if out of range.
     */
    FnFileSequenceHandle (*getFileSequence)(FnFileSequenceArrayHandle handle,
                                            unsigned int index,
                                            FnAttributeHandle *errorMessage);

    /** @brief Get array of filenames not belonging to a sequence.
     * @param handle A valid file sequence array handle.
     *
     * @return New handle (caller owns reference) to StringAttribute containing list of filenames.
     */
    FnAttributeHandle (*getFilenames)(FnFileSequenceArrayHandle handle,
                                      FnAttributeHandle *errorMessage);
};

#define kFnAssetFieldName                  "name"
#define kFnAssetFieldVersion               "version"

#define kFnAssetTypeKatanaScene            "katana scene"
#define kFnAssetTypeMacro                  "macro"
#define kFnAssetTypeLiveGroup              "live group"
#define kFnAssetTypeImage                  "image"
#define kFnAssetTypeLookFile               "look file"
#define kFnAssetTypeLookFileMgrSettings    "look file manager settings"
#define kFnAssetTypeAlembic                "alembic"
#define kFnAssetTypeScenegraphXml          "scenegraph xml"
#define kFnAssetTypeCastingSheet           "casting sheet"
#define kFnAssetTypeAttributeFile          "attribute file"
#define kFnAssetTypeFCurveFile             "fcurve file"
#define kFnAssetTypeGafferRig              "gaffer rig"
#define kFnAssetTypeGafferThreeRig         "gafferthree rig"
#define kFnAssetTypeScenegraphBookmarks    "scenegraph bookmarks"
#define kFnAssetTypeShader                 "shader"

#define kFnAssetRelationArgsFile           "related args file"

#define kFnAssetContextKatanaScene         "katana scene context"
#define kFnAssetContextMacro               "macro context"
#define kFnAssetContextLiveGroup           "live group context"
#define kFnAssetContextImage               "image context"
#define kFnAssetContextLookFile            "look file context"
#define kFnAssetContextLookFileMgrSettings "look file manager settings context"
#define kFnAssetContextAlembic             "alembic context"
#define kFnAssetContextScenegraphXml       "scenegraph xml context"
#define kFnAssetContextCastingSheet        "casting sheet context"
#define kFnAssetContextAttributeFile       "attribute file context"
#define kFnAssetContextFCurveFile          "fcurve file context"
#define kFnAssetContextGafferRig           "gaffer rig context"
#define kFnAssetContextGafferThreeRig      "gafferthree rig context"
#define kFnAssetContextScenegraphBookmarks "scenegraph bookmarks context"
#define kFnAssetContextShader              "shader context"
#define kFnAssetContextCatalog             "catalog context"
#define kFnAssetContextFarm                "farm context"

/** @{ */
/**
 * Key in the `args` dictionary passed to an asset plug-in's
 * createAssetAndPath() and postCreateAsset() parameters in some situations, in
 * particular when an asset is written out by a LookFileBake node. Supported
 * values are kFnAssetOutputFormatArchive or kFnAssetOutputFormatDirectory.
 */
#define kFnAssetCreationOptionOutputFormat "outputFormat"

/** The asset is being created as a single-file archive. */
#define kFnAssetOutputFormatArchive        "as archive"

/** The asset is being created as a directory. */
#define kFnAssetOutputFormatDirectory      "as directory"
/** @} */

/** @brief Blind declaration of object implementing FnAssetPluginSuite_v1
 */
typedef struct FnAssetStruct * FnAssetHandle;

/** @brief Blind declaration of a transaction object required for grouping actions in FnAssetPluginSuite_v1
 */
typedef struct FnAssetTransactionStruct * FnAssetTransactionHandle;


#define FnAssetPluginSuite_version 1

/** @brief Asset plugin suite
 *
 * This suite provides functions an asset system plugin must provide for creating and resolving assets.
 *
 * General design
 * An asset, as used in Katana, is a reference to input/output data --usually files on disk.  The reference
 * typically resolves to a file path or sequence on disk, but this is not a requirement.  It could resolve
 * to a colorspace name or shader library version string, for instance.  Scene files, input geometry/images,
 * look files, and more can be referenced as assets in Katana.  Although all these inputs and outputs can be
 * specified directly as file system paths, asset systems exist to provide higher-level tracking, versioning,
 * and publishing of this data than the file system alone provides.
 *
 * Assumptions about asset systems
 *  - Asset references can be uniquely serialized to/from a single string value (referred to as an assetId).
 *    In Imageworks' Vnp system, this is the spref url syntax(e.g. {spref:product.type?v=current}), but could
 *    just as easily be a guid or a serialized python dict.
 *  - Each asset has at least two levels of hierarchy: the basic asset container, which may contain multiple
 *    asset versions.
 *  - Each asset reference has some basic required fields associated with it (name and version, currently).
 */
struct FnAssetPluginSuite_v1
{
    FnAssetHandle (*create)();

    void (*destroy)(FnAssetHandle handle);

    /** @brief Reset will be called when Katana flushes its caches, giving the plugin a chance to reset.
     */
    void (*reset)(FnAssetHandle handle, FnAttributeHandle *errorMessage);

    /** @brief Return whether the input string represents a valid asset id.
     *
     * Should parse the input string to determine whether it is a valid asset id.  Should not attempt to
     * determine whether the asset actually exists in the asset database.
     *
     * @param str The string to test.
     *
     * @return True if the string is valid asset id syntax.
     */
    bool (*isAssetId)(FnAssetHandle handle, const char * str, FnAttributeHandle *errorMessage);

    /** @brief Return whether the input string contains a valid asset id anywhere within it.
     *
     * Should parse the input string to determine whether it contains a valid asset id.  Should not attempt
     * to determine whether any asset ids found in the string actually exist in the asset database.
     *
     * @param str The string to test.
     *
     * @return True if the string contains a valid asset id.
     */
    bool (*containsAssetId)(FnAssetHandle handle, const char * str, FnAttributeHandle *errorMessage);

    /** @brief Returns whether permissions for the given asset id are valid in the given context.
     *
     * @param  assetId Asset id to check permissions for.
     * @param  context Handle for a GroupAttribute used to specify more details about permissions to be checked.
     *
     * @return True if permissions for the given asset id are valid in the given context.
     */
    bool (*checkPermissions)(FnAssetHandle handle, const char *assetId, FnAttributeHandle context,
                             FnAttributeHandle *errorMessage);

    /** @brief Runs a custom command for the given asset id.
     *
     * @param  assetId Asset id the command will be run on.
     * @param  command Name of the command to run.
     * @param  commandArgs Handle for a GroupAttribute representing the command arguments.
     *
     * @return True if the command execution succeeds.
     */
    bool (*runAssetPluginCommand)(FnAssetHandle handle, const char *assetId, const char *command,
                                  FnAttributeHandle commandArgs, FnAttributeHandle *errorMessage);

    /** @brief Lookup asset id in asset system and return path (or other string) that it references.
     *
     * @param assetId Asset id to resolve.
     *
     * @return New handle (caller owns reference) to StringAttribute containing resolved asset string.
     *         On error, log error and return input string.
     */
    FnAttributeHandle (*resolveAsset)(FnAssetHandle handle, const char *assetId, FnAttributeHandle *errorMessage);

    /** @brief Replace any asset ids found in input string with resolved asset strings.
     *
     * @param str Input string to resolve.
     *
     * @return New handle (caller owns reference) to StringAttribute containing resolved asset string.
     *         On error, log error and return input string.
     */
    FnAttributeHandle (*resolveAllAssets)(FnAssetHandle handle, const char *str, FnAttributeHandle *errorMessage);

    /** @brief Resolve env vars in input path string, then resolve asset ids and file sequences.
     *
     * File sequence will most likely be resolved using default file sequence plugin from host.
     * This is a good candidate for some base functionality in the C++ wrappers on the plugin side.
     *
     * @param path Input path string to resolve.
     * @param frame Frame number to resolve file sequences with.
     *
     * @return New handle (caller owns reference) to StringAttribute containing resolved path string.
     *         On error, log error and return input string.
     */
    FnAttributeHandle (*resolvePath)(FnAssetHandle handle, const char *path, int frame, FnAttributeHandle *errorMessage);

    /** @brief Return integer version that this asset id resolves to.
     *
     * @param assetId Input asset id resolve.
     *
     * @return Integer version number that this asset id resolves to.
     *         On error, log error and return 0.
     */
    FnAttributeHandle (*resolveAssetVersion)(FnAssetHandle handle, const char *assetId, const char *versionStr,
                                             FnAttributeHandle *errorMessage);


    /** @brief: Returns a string that can be used in GUIs, for example.
     *
     * @param assetId The asset for which the display name is required
     *
     * @return New handle (caller owns reference) to StringAttribute containing display name
     *
     */
    FnAttributeHandle (*getAssetDisplayName)(FnAssetHandle handle, const char *assetId, FnAttributeHandle *errorMessage);

    /** @brief: Returns an array of null terminated strings listing the available versions for the given asset.
     *
     * @param assetId The asset who's versions we want
     *
     * @param versions The array of version strings to return
     */
    FnAttributeHandle (*getAssetVersions)(FnAssetHandle handle, const char *assetId, FnAttributeHandle *errorMessage);

    /** @brief Return a valid scenegraph location path that uniquely represents the input asset id.
     *
     * This will be used as the default location for look file materials loaded into
     * the Katana scene, for example.  (Example: /name/version/other/fields).
     *
     * @param assetId Input asset id to represent as scenegraph location path.
     *
     * @return New handle (caller owns reference) to StringAttribute containing scenegraph location path.
     */
    FnAttributeHandle (*getUniqueScenegraphLocationFromAssetId)(FnAssetHandle handle, const char *assetId,
                                                                bool includeVersion, FnAttributeHandle *errorMessage);

    /** @brief Return asset id that is related to input asset, given a relationship type.
     *
     * A general function for getting related assets.  An current example in Katana is
     * an "argsxml" asset defining the UI for a renderer procedural asset.
     *
     * @param assetId Input asset id for which to find related asset.
     * @param relation String representing relationship type.  (We might want to add #defines for these).
     *
     * @return New handle (caller owns reference) to StringAttribute containing related assed id (or empty string if
     *         no related asset is found).
     */
    FnAttributeHandle (*getRelatedAssetId)(FnAssetHandle handle, const char *assetId, const char *relation,
                                           FnAttributeHandle *errorMessage);

    /** @brief Convert from asset id to a set of named string fields defining the asset.
     *
     * The minimum set of fields are "name" and "version".  Other fields are preserved but ignored by Katana.
     *
     * @param assetId Input asset id.
     * @param includeDefaults If true, return the full set of fields for this asset system, including defaults not
     *        specified in the asset id.  If false, return only the fields specified in the asset id.
     *
     * @return New handle (caller owns reference) to GroupAttribute containing StringAttributes representing asset fields.
     *         On error, return empty GroupAttribute.
     */
    FnAttributeHandle (*getAssetFields)(FnAssetHandle handle, const char *assetId, bool includeDefaults,
                                        FnAttributeHandle *errorMessage);

    /** @brief Convert from asset fields to asset id
     *
     * The inverse of getAssetFields, this takes a set of asset fields and returns an asset id string.
     *
     * @param fields GroupAttribute containing StringAttributes representing asset fields.
     *
     * @return New handle (caller owns reference) to StringAttribute containing asset id.
     */
    FnAttributeHandle (*buildAssetId)(FnAssetHandle handle, FnAttributeHandle fields, FnAttributeHandle *errorMessage);

    /** @brief Get metadata associated with an asset or a scoped item in the asset hierarchy.
     *
     * This differs from the above "asset fields" which are the fields that uniquely define
     * the asset itself.  This metadata may include fields such as asset creator, creation time, etc.
     *
     * @param assetId Asset for which to return metadata.
     * @param scope Optional string specifying scope for metadata lookup, such as "name" or "version".
     *
     * @return New handle (caller owns reference) to GroupAttribute containing StringAttributes representing metadata attributes.
     */
    FnAttributeHandle (*getAssetAttributes)(FnAssetHandle handle, const char *assetId, const char *scope,
                                            FnAttributeHandle *errorMessage);

    /** @brief Set metadata associated with an asset or a scoped item in the asset hierarchy.
     *
     * This differs from the above "asset fields" which are the fields that uniquely define
     * the asset itself.  This metadata may include fields such as asset creator, creation time, etc.
     *
     * @param assetId Asset for which to set metadata.
     * @param scope Optional string specifying scope for metadata lookup, such as "name" or "version".
     */
    void (*setAssetAttributes)(FnAssetHandle handle, const char *assetId, const char *scope,
                               FnAttributeHandle attributes, FnAttributeHandle *errorMessage);

    /** @brief Get asset id scoped to the specified level in the asset hierarchy.
     *
     * Should return a more general asset reference to the given asset id.  For instance,
     * if scope is "name", return an asset id that doesn't include a specific version number.
     * This may not make sense in some asset systems, in which case, return the input asset id.
     *
     * @param assetId Input asset id.
     * @param scope String specifying scope for result asset id, such as "name" or "version".
     *
     * @return New handle (caller owns reference) to StringAttribute containing result asset id.
     */
    FnAttributeHandle (*getAssetIdForScope)(FnAssetHandle handle, const char *assetId, const char *scope,
                                            FnAttributeHandle *errorMessage);

    /** @brief Create a transaction object.
     *
     * Transaction objects are used to group asset/directory creation actions to allow them to
     * succeed or fail as a unit.
     *
     * @return New handle (caller owns handle) to transaction object.  If asset plugin does not
     *         support transactions, always return 0x0.
     */
    FnAssetTransactionHandle (*createTransaction)(FnAssetHandle handle, FnAttributeHandle *errorMessage);

    /** @brief Cancel transaction and destroy transaction object.
     *
     * @param handle Handle to transaction object.
     */
    void (*cancelTransaction)(FnAssetTransactionHandle handle, FnAttributeHandle *errorMessage);

    /** @brief Commit transaction and destroy transaction object.
     *
     * @param handle Handle to transaction object.
     *
     * @return True if transaction successfully commited.
     */
    bool (*commitTransaction)(FnAssetTransactionHandle handle, FnAttributeHandle *errorMessage);

    /** @brief Create asset and optional directory path.
     *
     * @param txn Handle to transaction object.  If null, asset/directory creation is done immediately.
     * @param assetType Type of Katana asset to create (e.g. katana scene, image, shadow, etc)
     * @param assetFields GroupAttribute representing asset fields for asset to create (same as getAssetFields output).
     * @param args Additional args used to specify more details (may differ based on assetType).  Examples include
     *        "versionUp", which is a boolean determining whether to version up an existing asset.
     * @param createDirectory Boolean specifying whether to create the directory associated with this asset.
     *
     * @return New handle (caller owns reference) to StringAttribute containing asset id created (or that
     *         will be created on transaction commit).  On error, return valid handle to StringAttribute
     *         containing empty string.
     */
    FnAttributeHandle (*createAssetAndPath)(FnAssetHandle handle, FnAssetTransactionHandle txn,
            const char *assetType, FnAttributeHandle assetFields, FnAttributeHandle args,
            bool createDirectory, FnAttributeHandle *errorMessage);

    /** @brief Creates an asset after the creation of the files to be published.
     *
     * @param txn Handle to transaction object.  If null, asset/directory creation is done immediately.
     * @param assetType Type of Katana asset to create (e.g. katana scene, image, shadow, etc)
     * @param assetFields GroupAttribute representing asset fields for asset to create (same as getAssetFields output).
     * @param args Additional args used to specify more details (may differ based on assetType).  Examples include
     *        "versionUp", which is a boolean determining whether to version up an existing asset.
     * @param createDirectory Boolean specifying whether to create the directory associated with this asset.
     *
     * @return New handle (caller owns reference) to StringAttribute containing asset id created (or that
     *         will be created on transaction commit).  On error, return valid handle to StringAttribute
     *         containing empty string.
     */
    FnAttributeHandle (*postCreateAsset)(FnAssetHandle handle, FnAssetTransactionHandle txn,
            const char *assetType, FnAttributeHandle assetFields, FnAttributeHandle args,
            FnAttributeHandle *errorMessage);
};


#define FnAssetHostSuite_version 3

/** Plugins may want to access the asset or file sequence plugins available in the host for
 * resolving file paths, etc.  An example might be a ScenegraphGenerator plugin that needs to
 * resolve assets pointing at data file inputs within the hierarchy it is building.

 * We deliberately use a set of simplified functions provided by the host instead of
 * exposing the AssetPlugin/FileSequencePlugin suites directly.  This allows us to change
 * either suite in a future update without breaking compatibility with plugins using the
 * other suite.
 */
struct FnAssetHostSuite_v1
{
    bool (*isAssetId)(const char * str, FnAttributeHandle *errorMessage);
    bool (*containsAssetId)(const char * str, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveAsset)(const char *assetId, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolvePath)(const char *path, int frame, FnAttributeHandle *errorMessage);
    bool (*isFileSequence)(const char *path, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*buildFileSequenceString)(const char *prefix, const char *suffix, int padding, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveFileSequence)(const char *path, int frame, FnAttributeHandle *errorMessage);
    bool (*isFrameInFileSequence)(const char *path, int frame, FnAttributeHandle *errorMessage);
};

struct FnAssetHostSuite_v2
{
    bool (*isAssetId)(const char * str, FnAttributeHandle *errorMessage);
    bool (*containsAssetId)(const char * str, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveAsset)(const char *assetId, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolvePath)(const char *path, int frame, FnAttributeHandle *errorMessage);
    bool (*isFileSequence)(const char *path, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*buildFileSequenceString)(const char *prefix, const char *suffix,
            int padding, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveFileSequence)(const char *path, int frame, FnAttributeHandle *errorMessage);
    bool (*isFrameInFileSequence)(const char *path, int frame, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getUniqueScenegraphLocationFromAssetId)(const char *assetId,
            bool includeVersion, FnAttributeHandle *errorMessage);

    FnAttributeHandle (*getRelatedAssetId)(const char *assetId,
            const char *relation, FnAttributeHandle *errorMessage);


};

struct FnAssetHostSuite_v3
{
    bool (*isAssetId)(const char * str, FnAttributeHandle *errorMessage);
    bool (*containsAssetId)(const char * str, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveAsset)
        (const char *assetId, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolvePath)
        (const char *path, int frame, FnAttributeHandle *errorMessage);
    bool (*isFileSequence)
        (const char *path, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*buildFileSequenceString)
        (const char *prefix, const char *suffix,
         int padding, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveFileSequence)
        (const char *path, int frame, FnAttributeHandle *errorMessage);
    bool (*isFrameInFileSequence)
        (const char *path, int frame, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getUniqueScenegraphLocationFromAssetId)
        (const char *assetId, bool includeVersion,
         FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getRelatedAssetId)(const char *assetId,
            const char *relation, FnAttributeHandle *errorMessage);
    // v3 functions
    FnAttributeHandle (*getAssetAttributes)
        (const char *assetId, const char *scope,
         FnAttributeHandle *errorMessage);
    bool (*checkPermissions)
        (const char *assetId, const char **context, size_t count,
         FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveAllAssets)
        (const char *assetId, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*resolveAssetVersion)
        (const char *assetId, const char *versionTag,
         FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getAssetDisplayName)
        (const char *assetId, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getAssetVersions)
        (const char *assetId, FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getAssetFields)
        (const char *assetId, bool includeDefaults,
         FnAttributeHandle *errorMessage);
    FnAttributeHandle (*buildAssetId)
        (const char **fields, size_t fieldsCount,
         FnAttributeHandle *errorMessage);
    FnAttributeHandle (*getAssetIdForScope)
        (const char *assetId, const char *scope,
         FnAttributeHandle *errorMessage);
};
/** @} */

typedef FnAssetHostSuite_v3 FnAssetHostSuite;

///@endcond

}

#endif // FnAssetSuite_H
