// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNDEFAULTFILESEQUENCEPLUGIN__H
#define FNDEFAULTFILESEQUENCEPLUGIN__H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAsset/suite/FnAssetSuite.h>

#include <FnAttribute/FnAttribute.h>

#include "ns.h"

#include <vector>
#include <string>
#include <memory>

FNASSET_NAMESPACE_ENTER
{
    class FileSequenceArray;

    /**
     * \defgroup FnAsset Asset Host API
     * @{
     *
     *
     */

    /**
     * @brief Allows the access to the currently set File Sequence plugin.
     *
     * This class can be used when a plugin (a Scenegraph Generator,
     * for example) needs to do build or resolve a file sequence using the
     * File Sequence Plugin set in the current Katana scene.
     */
    class DefaultFileSequencePlugin
    {
    public:

        /** @brief Return true if string is a valid file sequence.
         *  @param path The string to test.
         *  @param throwOnError Boolean setting whether to throw an exception on error.
         */
        static bool isFileSequence(const std::string& path, bool throwOnError=false);

        /** @brief Return a valid file sequence string built from input prefix/suffix/padding

         *
         *  @param prefix A string with the portion of the sequence path before the frame symbol(s)
         *  @param sufix A string with the portion of the sequence path after the frame symbol(s)
         *  @padding The size of the zero padding for the frame number
         *  @param throwOnError Boolean setting whether to throw an exception on error.
         */
        static std::string buildFileSequenceString(const std::string& prefix, const std::string& suffix,
                                                   int padding, bool throwOnError=false);

        /** @brief Resolves a file sequence string into an single real file path
         *
         * Given a sequence path string and a frame number, it returns the actual
         * file path for that frame.
         *
         *  @param path The sequence path string.
         *  @param frame The frame number.
         *  @param throwOnError Boolean setting whether to throw an exception on error.
         */
        static std::string resolveFileSequence(const std::string& path, int frame,
                                               bool throwOnError=false);

        /** @brief Returns whether or not a given frame is a part of the file sequence
         *
         * @param path The sequence path string
         * @param frame The frame number
         * @param throwOnError Boolean setting whether to throw an exception on error.
         * @return true if the frame is a part of the file sequence, false otherwise
         */
        static bool isFrameInFileSequence(const std::string& path, int frame,
                                          bool throwOnError=false);


        ///@cond FN_INTERNAL_DEV
        static FnPlugStatus setHost(FnPluginHost *host);

    private:
        static FnAssetHostSuite_v2 *_hostSuite;

        ///@endcond
    };

    /** @} */

}
FNASSET_NAMESPACE_EXIT

#endif // FNDEFAULTFILESEQUENCEPLUGIN__H
