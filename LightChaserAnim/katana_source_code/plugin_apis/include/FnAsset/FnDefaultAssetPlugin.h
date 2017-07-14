// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNDEFAULTASSETPLUGIN__H
#define FNDEFAULTASSETPLUGIN__H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAsset/suite/FnAssetSuite.h>

#include "ns.h"

#include <string>

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
    class DefaultAssetPlugin
    {
    public:

        /** @brief Return whether the input string represents a valid asset id.
         *
         * Should parse the input string to determine whether it is a valid
         * asset id. Should not attempt to determine whether the asset actually
         * exists in the asset database.
         *
         * @param str The string to test.
         * @param bool Boolean setting whether to throw an exception on error.
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
         * @param bool Boolean setting whether to throw an exception on error.
         *
         * @return True if the string contains a valid asset id.
         */
        static bool containsAssetId(const std::string& str,
                                    bool throwOnError=false);


        /** @brief Lookup asset id in asset system and return path (or other
         * string) that it references.
         *
         * @param assetId Asset id to resolve.
         * @param bool Boolean setting whether to throw an exception on error.
         * @return The resolved asset string.
         */
        static std::string resolveAsset(const std::string& assetId,
                                        bool throwOnError=false);


        /**
         * @brief Resolve env vars in input path string, then resolve asset ids
         * and file sequences.
         *
         * File sequence will most likely be resolved using default file sequence
         * plugin from host. This is a good candidate for some base functionality
         * in the C++ wrappers on the plugin side.
         *
         * @param path Input path string to resolve.
         * @param frame Frame number to resolve file sequences with.
         * @param bool Boolean setting whether to throw an exception on error.
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


        ///@cond FN_INTERNAL_DEV
        static FnPlugStatus setHost(FnPluginHost *host);

    private:
        static const FnAssetHostSuite_v2 *_hostSuite;

        ///@endcond
    };

    /** @} */
}
FNASSET_NAMESPACE_EXIT

#endif  // FNDEFAULTASSETPLUGIN__H
