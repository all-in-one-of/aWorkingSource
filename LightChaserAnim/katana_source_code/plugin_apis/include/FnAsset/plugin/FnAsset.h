// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNASSET__H
#define FNASSET__H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAsset/suite/FnAssetSuite.h>

#include <FnAttribute/FnAttribute.h>

#include <string>
#include <memory>
#include <map>

namespace Foundry
{
  namespace Katana
  {
    class AssetTransaction;

    /**
     * \defgroup FnAsset Asset Plug-In API
     * @{
     *
     *
     * This API allows the implementation of two plugin types:
     *   - %Asset Plugin - which implements the connection of Katana with an
     *     %Asset Management System (see Asset).
     *
     *   - File Sequence Plugin - which tells Katana how a fileSequence should
     *     be described as a string, and how to resolve that string into a real
     *     file path when given a frame number (see FileSequence).
     *
     *  Each Katana Scene will have one %Asset plugin and one %FileSequence plugin associated with it
     *  (see in the Katana UI: Project Settings tab-> plugins -> asset / fileSequence)
     *  Which can be accessed from other plugins using the classes DefaultAssetPlugin and DefaultFileSequencePlugin
     *
     *  Both Plugins can also be implemented in Python with a very similar interface
     *  (see the documentation for the AssetAPI Python module). Python plugins will
     *  be accessed by Katana and by any plugin using the DefaultAssetPlugin and
     *  DefaultFileSequencePlugin classes in a transparent way.
     *
     *  <h3>General design</h3>
     *  An asset, as used in Katana, is a reference to input/output data -- usually files on disk.  The reference
     *  typically resolves to a file path or sequence on disk, but this is not a requirement.  It could resolve
     *  to a colorspace name or shader library version string, for instance.  Scene files, input geometry/images,
     *  look files, and more can be referenced as assets in Katana.  Although all these inputs and outputs can be
     *  specified directly as file system paths, asset systems exist to provide higher-level tracking, versioning,
     *  and publishing of this data than the file system alone provides.
     *
     *  <h3>Assumptions about asset systems</h3>
     *  - A reference to an asset can be uniquely serialized to/from a single string value (referred to as an <b>AssetId</b>).
     *    In Imageworks' Vnp system, this is the spref url syntax(e.g. {spref:product.type?v=current}), but could
     *    just as easily be a guid or a serialized python dict.
     *  - Each asset has at least two levels of hierarchy: the basic asset container, which may contain multiple
     *    asset versions.
     *  - Each asset reference has some basic required <b>Fields</b> associated with it ("name" and "version" are
     *    currently the only two mandatory fields for every asset).
     *  - Each asset can have a set of non-mandatory <b>Attributes</b> (name/value pairs)
     *    in which some meta-data related with the asset can be stored or used
     *    by the %Asset Management System (Example: an image asset might have its
     *    LUT name, file type and resolution associated with it)
     *
     *
     *  <h3>File Sequence Plugins</h3>
     *
     *  File sequence Plugins allow to set the way file sequence paths are
     *  represented and resolved by Katana. Check the documentation about the
     *  FileSequence class for more information.
     *
     */

    /**
     * @brief Primary class to derive from when implementing asset system plugin.
     *
     * This is a virtual base class to be extended in your plugin.
     *
     * In addition to implementing all the pure virtual functions in this
     * class, you must also implement a static method in your derived class:
     *
     * \code
     * // return a new instance of your derived class, owned by caller.
     * static Asset *create();
     * \endcode
     *
     * To indicate an error to the caller, your implemntation may throw an
     * exception derived from std::exception.
     */
    class Asset
    {
    public:
      typedef std::map<std::string, std::string> StringMap;
      typedef std::vector<std::string> StringVector;

      Asset() { /* Empty */ }
      virtual ~Asset() { /* Empty */ }

      /** @brief Reset will be called when Katana flushes its caches, giving the plugin a chance to reset.
       */
      virtual void  reset() = 0;

      /** @brief Return whether the input string represents a valid asset id.
       *
       * Should parse the input string to determine whether it is a valid asset id.  Should not attempt to
       * determine whether the asset actually exists in the asset database.
       *
       * @param str The string to test.
       *
       * @return True if the string is valid asset id syntax.
       */
      virtual bool  isAssetId(const std::string& str) = 0;

      /** @brief Return whether the input string contains a valid asset id anywhere within it.
       *
       * Should parse the input string to determine whether it contains a valid asset id.  Should not attempt
       * to determine whether any asset ids found in the string actually exist in the asset database.
       *
       * @param str The string to test.
       *
       * @return True if the string contains a valid asset id.
       */
      virtual bool  containsAssetId(const std::string& str) = 0;

      /** @brief Returns whether permissions for the given asset id are valid in the given context.
       *
       * @param  assetId Asset id to check permissions for.
       * @param  context Additional strings used to specify more details about permissions to be checked.
       *         Examples include current user or current show environment.
       *
       * @return True if permissions for the given asset id are valid in the given context.
       */
      virtual bool checkPermissions(const std::string &assetId, const StringMap &context) = 0;

      /** @brief Runs a custom command for the given asset id.
       *
       * @param  assetId Asset id the command will be run on.
       * @param  command Name of the command to run.
       * @param  commandArgs Key-string pair representing the command arguments.
       *
       * @return True if the command execution succeeds.
       */
      virtual bool runAssetPluginCommand(const std::string &assetId, const std::string &command, const StringMap &commandArgs) = 0;

      /** @brief Lookup asset id in asset system and return path (or other string) that it references.
       *
       * @param assetId Asset id to resolve.
       * @param ret Set to resolved asset string.
       */
      virtual void  resolveAsset(const std::string& assetId, std::string& ret) = 0;

      /** @brief Replace any asset ids found in input string with resolved asset strings.
       *
       * @param str Input string to resolve.
       *
       * @param ret Set to resolved asset string.
       */
      virtual void  resolveAllAssets(const std::string& str, std::string& ret) = 0;

      /** @brief Resolve env vars in input path string, then resolve asset ids and file sequences.
       *
       * File sequence will most likely be resolved using default file sequence plugin from host.
       * This is a good candidate for some base functionality in the C++ wrappers on the plugin side.
       *
       * @param str Input path string to resolve.
       * @param frame Frame number to resolve file sequences with.
       *
       * @param ret Set to resolved path string.
       */
      virtual void  resolvePath(const std::string& str, const int frame, std::string& ret) = 0;

      /** @brief Return the version that this asset id resolves to.
       *
       * @param assetId Input asset id resolve.
       *
       * @param ret Returns the version that this asset id resolves to.
       *
       * @param versionStr In some asset management systems the version can be
       *                   specified by keywords (example: latest, aproved, etc)
       *                   that might be associated to a specific version number.
       *                   If versionStr is specified, then the plugin will try
       *                   to return (on ret) the resolved string number for the
       *                   assetId specified by str and the given version keyword
       *                   (the versionStr value).
       */
      virtual void  resolveAssetVersion(const std::string& assetId,
        std::string& ret, const std::string& versionStr = std::string()) = 0;

      /** @brief Return a valid scenegraph location path that uniquely represents the input asset id.
       *
       * This will be used as the default location for look file materials loaded into
       * the Katana scene, for example.  (Example: /name/version/other/fields).
       *
       * @param assetId Input asset id to represent as scenegraph location path.
       *
       * @param includeVersion: allows to specify if the scenegraph location path should contain the asset version
       *
       * @param ret Set to scenegraph location path.
       */
      virtual void  getUniqueScenegraphLocationFromAssetId(const std::string& assetId,
              bool includeVersion, std::string& ret) = 0;

      /** @brief: Returns a string that can be used in GUIs, for example.
       *
       * @param assetId The asset for which the display name is required
       *
       * @param ret Return value with the display name.
       */
      virtual void getAssetDisplayName(const std::string& assetId, std::string& ret) = 0;

      /** @brief: Returns a vector of strings listing the available versions for the given asset.
       *
       * @param assetId The asset who's versions we want
       *
       * @param ret The string vector of available versions
       */
      virtual void getAssetVersions(const std::string& assetId, StringVector& ret) = 0;

      /** @brief Return asset id that is related to input asset, given a relationship type.
       *
       * A general function for getting related assets.  An current example in Katana is
       * an "argsxml" asset defining the UI for a renderer procedural asset.
       *
       * @param assetId Input asset id for which to find related asset.
       * @param relation String representing relationship type.
       *
       * @param ret Set to related assed id (or empty string if no related asset is found).
       */
      virtual void  getRelatedAssetId(const std::string& assetId, const std::string& relation,
        std::string& ret) = 0;

      /** @brief Convert from asset id to a set of named string fields defining the asset.
       *
       * The minimum set of fields are "name" and "version".  Other fields are preserved but ignored by Katana.
       *
       * @param assetId Input asset id.
       * @param includeDefaults If true, return the full set of fields for this asset system, including defaults not
       *        specified in the asset id.  If false, return only the fields specified in the asset id.
       *
       * @param returnFields Fill map with key/value strings representing asset fields.
       */
      virtual void  getAssetFields(const std::string& assetId, bool includeDefaults,
        StringMap& returnFields) = 0;

      /** @brief Convert from asset fields to asset id
       *
       * The inverse of getAssetFields, this takes a set of asset fields and returns an asset id string.
       *
       * @param fields String map containing key/value strings representing asset fields.
       *
       * @param ret Set to asset id.
       */
      virtual void  buildAssetId(const StringMap& fields, std::string& ret) = 0;

      /** @brief Get metadata associated with an asset or a scoped item in the asset hierarchy.
       *
       * This differs from the above "asset fields" which are the fields that uniquely define
       * the asset itself.  This metadata may include fields such as asset creator, creation time, etc.
       *
       * @param assetId Asset for which to return metadata.
       * @param scope Optional string specifying scope for metadata lookup, such as "name" or "version".
       *
       * @param returnAttrs Fill with key/value strings representing metadata attributes.
       */
      virtual void  getAssetAttributes(const std::string& assetId, const std::string& scope,
        StringMap& returnAttrs) = 0;

      /** @brief Set metadata associated with an asset or a scoped item in the asset hierarchy.
       *
       * This differs from the above "asset fields" which are the fields that uniquely define
       * the asset itself.  This metadata may include fields such as asset creator, creation time, etc.
       *
       * @param assetId Asset for which to set metadata.
       * @param scope Optional string specifying scope for metadata lookup, such as "name" or "version".
       * @param attrs String map containing key/value strings representing metadata attributes.
       */
      virtual void  setAssetAttributes(const std::string& assetId, const std::string& scope,
        const StringMap& attrs) = 0;

      /** @brief Get partial asset id scoped to the specified level in the asset hierarchy.
       *
       * Should return a more general asset reference to the given asset id.  For instance,
       * if scope is "name", return an asset id that doesn't include a specific version number.
       * This may not make sense in some asset systems, in which case, return the input asset id.
       *
       * @param assetId Input asset id.
       * @param scope String specifying scope for result asset id, such as "name" or "version".
       *
       * @param ret Set to result asset id.
       */
      virtual void  getAssetIdForScope(const std::string& assetId, const std::string& scope,
        std::string& ret) = 0;

      /** @brief Create asset and optional directory path.
       *
       * @param txn Handle to transaction object.  If null, asset/directory creation is done immediately.
       * @param assetType Type of Katana asset to create (e.g. katana scene, image, shadow, etc)
       * @param assetFields GroupAttribute representing asset fields for asset to create (same as getAssetFields output).
       * @param args Additional args used to specify more details (may differ based on assetType).  Examples include
       *        "versionUp", which is a boolean determining whether to version up an existing asset or "colorspace"
       *        if the assetId is of the type "image" (in which if this is provided it should be seen as an Attribute
       *        to be associated with the asset)
       * @param createDirectory Boolean specifying whether to create the directory associated with this asset.
       *
       * @param assetId Set to the asset id created (or that will be created on transaction commit).
       */
      virtual void  createAssetAndPath(AssetTransaction* txn, const std::string& assetType,
        const StringMap& assetFields, const StringMap& args, bool createDirectory,
        std::string& assetId) = 0;

      /** @brief Creates an asset after the creation of the files to be published.
       *
       * @param txn Handle to transaction object.  If null, asset/directory creation is done immediately.
       * @param assetType Type of Katana asset to create (e.g. katana scene, image, shadow, etc)
       * @param assetFields GroupAttribute representing asset fields for asset to create (same as getAssetFields output).
       * @param args Additional args used to specify more details (may differ based on assetType).  Examples include
       *        "versionUp", which is a boolean determining whether to version up an existing asset.
       *
       * @param assetId Set to the asset id created (or that will be created on transaction commit).
       */
      virtual void  postCreateAsset(AssetTransaction* txn, const std::string& assetType,
              const StringMap& assetFields, const StringMap& args,
              std::string& assetId) = 0;


      /** @brief Create a transaction object.
       *
       * Transaction objects are used to group asset/directory creation actions to allow them to
       * succeed or fail as a unit.
       *
       * @return Pointer to heap-allocated instance of derived AssetTransaction.  Caller owns
       *         returned instance.  If asset plugin does not support transactions, always return 0x0.
       */
      virtual AssetTransaction * createTransaction();

      ///@cond FN_INTERNAL_DEV
      static FnPlugStatus             setHost(FnPluginHost* host);
      static FnPluginHost*            getHost();
      static FnAssetPluginSuite_v1    createSuite(FnAssetHandle (*create)());

      static FnAssetHandle            newAssetHandle(Asset* asset);
      static FnAssetTransactionHandle newAssetTransactionHandle(AssetTransaction* txn);

      static unsigned int           _apiVersion;
      static const char*            _apiName;

    private:
      static FnPluginHost*          _host;

      ///@endcond
    };

    /**
     * @brief Virtual base for asset transactions.
     *
     * Derive from this in your plugin to support transactions.  A SimpleTransaction class
     * is included here that maintains a list of actions to be executed on commit().
     */
    class AssetTransaction
    {
    public:
      AssetTransaction() {/* Empty */}
      virtual ~AssetTransaction() {/* Empty */}

      /** @brief Cancel transaction.
       */
      virtual void cancel() = 0;

      /** @brief Commit transaction.
       */
      virtual bool commit() = 0;
    };

    /**
     * @brief Simple action base class for performing an action (doit) and rolling it back (rollback)
     *        in case of error in this or subsequent action.
     */
    class SimpleAction
    {
    public:
      virtual ~SimpleAction() {}
      virtual bool doit() = 0;
      virtual void rollback() = 0;
    };

    /**
     * @brief Simple transaction maintains a list of actions.  If an action returns failure during commit,
     *        execution is canceled and rollback() is called on all previously executed actions in
     *        transaction.
     */
    class SimpleTransaction : public AssetTransaction
    {
    public:
      virtual ~SimpleTransaction();

      void addAction(SimpleAction *action);

      void cancel();

      bool commit();

    private:
      std::vector<SimpleAction *> _actions;

      void _clearActions();
    };

    /** @} */


  };  // namespace Katana
};  // namespace Foundry

namespace FnKat = Foundry::Katana;

///@cond FN_INTERNAL_DEV

#define DEFINE_ASSET_PLUGIN(ASSET_CLASS)                                    \
                                                                            \
  FnPlugin ASSET_CLASS##_plugin;                                            \
                                                                            \
  FnAssetHandle ASSET_CLASS##_create()                                      \
  {                                                                         \
    return Foundry::Katana::Asset::newAssetHandle(ASSET_CLASS::create());   \
  }                                                                         \
                                                                            \
  FnAssetPluginSuite_v1 ASSET_CLASS##_suite =                               \
    Foundry::Katana::Asset::createSuite(ASSET_CLASS##_create);              \
                                                                            \
  const void* ASSET_CLASS##_getSuite()                                      \
  {                                                                         \
    return &ASSET_CLASS##_suite;                                            \
  }                                                                         \


class FnAssetStruct
{
public:
  FnAssetStruct(Foundry::Katana::Asset* asset) :
    _asset(asset) { /* Empty */ }
  ~FnAssetStruct() { /* Empty */ }

  Foundry::Katana::Asset& getAsset()
  {
      return *_asset;
  }

private:
  std::auto_ptr<Foundry::Katana::Asset> _asset;
};

class FnAssetTransactionStruct
{
public:
  FnAssetTransactionStruct(Foundry::Katana::AssetTransaction* txn) :
    _assetTransaction(txn) { /* Empty */ }
  ~FnAssetTransactionStruct() { /* Empty */ }

  Foundry::Katana::AssetTransaction& getAssetTransaction()
  {
    return *_assetTransaction;
  }

private:
  std::auto_ptr<Foundry::Katana::AssetTransaction> _assetTransaction;
};

///@endcond

#endif  // #ifndef FNASSET__H
