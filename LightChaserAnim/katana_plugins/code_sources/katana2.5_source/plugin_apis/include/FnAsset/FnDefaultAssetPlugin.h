// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNDEFAULTASSETPLUGIN__H
#define FNDEFAULTASSETPLUGIN__H

#include <map>
#include <string>
#include <vector>

#include <FnAsset/FnAssetAPI.h>
#include <FnAsset/ns.h>
#include <FnAsset/suite/FnAssetSuite.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnPluginSystem/FnPluginSystem.h>

FNASSET_NAMESPACE_ENTER
{
    /**
     * \defgroup FnAsset Asset Host API
     * @{
     */

    /**
     *
     * @brief Allows the access to the currently set Asset Management System
     * plugin.
     *
     * This class can be used when a plugin (a Scenegraph Generator,
     * for example) needs to resolve or publish assets using the %Asset
     * Management System Plugin set on the current scene (regardless if it is
     * implemented in C++ or Python)....
     */
    class FNASSET_API DefaultAssetPlugin
    {
    public:

        /** @brief Return whether the input string represents a valid asset id.
         *
         * Should parse the input string to determine whether it is a valid
         * asset id. Should not attempt to determine whether the asset actually
         * exists in the asset database.
         *
         * @param str The string to test.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         *
         * @return True if the string is valid asset id syntax.
         */
        static bool isAssetId(const std::string& str, bool throwOnError=false);

        /** @brief Return whether the input string contains a valid asset id
         * anywhere within it.
         *
         * Should parse the input string to determine whether it contains a
         * valid asset id.  Should not attempt to determine whether any asset
         * ids found in the string actually exist in the asset database.
         *
         * @param str The string to test.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         *
         * @return True if the string contains a valid asset id.
         */
        static bool containsAssetId(const std::string& str,
                                    bool throwOnError=false);


        /** @brief Lookup asset id in asset system and return path (or other
         * string) that it references.
         *
         * @param assetId Asset id to resolve.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return The resolved asset string.
         */
        static std::string resolveAsset(const std::string& assetId,
                                        bool throwOnError=false);


        /**
         * Replace any asset ids found in the given input string with
         * resolved asset strings.
         *
         * @param inputString The string to be searched for asset ids.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return The string with all asset ids replaced with resolved asset
         *     strings.
         */
        static std::string resolveAllAssets(const std::string &inputString,
                                            bool throwOnError = false);

        /**
         * @brief Resolve env vars in input path string, then resolve asset ids
         * and file sequences.
         *
         * File sequence will most likely be resolved using default file
         * sequence plugin from host. This is a good candidate for some base
         * functionality in the C++ wrappers on the plugin side.
         *
         * @param path Input path string to resolve.
         * @param frame Frame number to resolve file sequences with.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         *
         * @return The resolved path string.
         */
        static std::string resolvePath(const std::string& path, int frame,
                                       bool throwOnError=false);


        static std::string getUniqueScenegraphLocationFromAssetId(
                const std::string & assetId, bool includeVersion,
                bool throwOnError=false);

        static std::string getRelatedAssetId(
                const std::string & assetId, const std::string& relation,
                bool throwOnError=false);

        /**
         * Gets metadata associated with an asset or a scoped item in the
         * asset hierarchy.
         *
         * @param assetId The id of the asset to search for metadata.
         * @param scope A scoped item name.
         * @param attributes A map to be filled with the metadata attributes
         *     of the given asset.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return
         */
        static bool getAssetAttributes(
            const std::string &assetId, const std::string &scope,
            std::map<std::string, std::string> &attributesMap,
            bool throwOnError = false);

        /**
         * Returns whether permissions for the given asset id are valid in the
         * given context.
         *
         * @param assetId The id of the asset to check permission for.
         * @param context
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return
         */
        static bool checkPermissions(
            const std::string &assetId,
            const std::map<std::string, std::string> &context,
            bool throwOnError = false);

        /**
         * Returns the version for a given asset id.
         * If it is a partial asset id (which does not have a version) then
         * an empty string is returned.
         *
         * @param assetId The asset id to resolve.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return String with the integer version number that this asset id
         *     resolves to, or an empty string if an error occurs.
         */
        static std::string resolveAssetVersion(
            const std::string &assetId,
            const std::string &versionTag = std::string(),
            bool throwOnError = false);

        /**
         * Returns a name suitable for displaying to the user for the given
         * asset id.
         *
         * @param assetId The asset id to look for display name.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return The display name of the given asset.
         */
        static std::string getAssetDisplayName(
            const std::string &assetId, bool throwOnError = false);

        /**
         * Gets a list of all the available versions of the given asset.
         *
         * @param assetId The asset id to look for available versions.
         * @param versions The vector to be filled with the available versions
         *     of the given asset.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return True on success.
         */
        static bool getAssetVersions(
            const std::string& assetId,
            std::vector<std::string> &versions,
            bool throwOnError = false);

        /**
         * Convert from asset id to a set of named string fields defining
         * the asset.
         *
         * The minimum set of fields are "name" and "version".
         * Other fields are preserved but ignored by Katana.
         *
         * @param assetId Input asset id.
         * @param includeDefaults If true, return the full set of fields for
         *     this asset system, including defaults not specified in the asset
         *     id. If false, return only the fields specified in the asset id.
         * @param returnFields A map to be filled with the asset fields.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return True on success.
         */
        static bool getAssetFields(
            const std::string &assetId, bool includeDefaults,
            std::map<std::string, std::string> &returnFields,
            bool throwOnError = false);

        /**
         * Builds and return a new asset id using the given fields.
         *
         * @param fields A map of fields used to build the new asset id.
         * @param throwOnError Boolean setting whether to throw an exception
         *     on error.
         * @return The new asset id, on success. Otherwise an empty string.
         */
        static std::string buildAssetId(
            const std::map<std::string, std::string> &fields,
            bool throwOnError = false);

        /** Gets an asset id scoped to the specified level in the asset
         *  hierarchy.
         *
         *  Should return a more general asset reference to the given asset id.
         *  For instance, if scope is "name", returns an asset id that doesn't
         *  include a specific version number.
         *  This may not make sense in some asset systems, in which case,
         *  return the input asset id.
         *
         * @param assetId The input asset id.
         * @param scope A string specifying scope for result asset id,
         *     such as "name" or "version".
         *
         * @return A string containing the result asset id.
         */
        static std::string getAssetIdForScope(
            const std::string &assetId, const std::string &scope,
            bool throwOnError = false);

        ///@cond FN_INTERNAL_DEV
        static FnPlugStatus setHost(FnPluginHost *host);

    private:
        static const FnAssetHostSuite *_hostSuite;

        ///@endcond
    };

    /** @} */
}
FNASSET_NAMESPACE_EXIT

#endif  // FNDEFAULTASSETPLUGIN__H
