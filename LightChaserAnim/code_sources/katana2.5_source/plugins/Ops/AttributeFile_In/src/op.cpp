// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include <FileReader.h>
#include <AttrDataCache.h>

namespace { //anonymous

class AttributeFile_InOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        // This Op can be run concurrently on multiple locations, so
        // provide ThreadModeConcurrent as the threading mode
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
        if (celAttr.isValid())
        {
            // If a CEL attribute was provided (and so it's valid), check
            // our current output location against the CEL. If it doesn't match
            // the CEL, then don't continue cooking at this location.

            // Use the utility function matchesCEL() to populate a
            // MatchesCELInfo object that contains useful attributes we can
            // query
            FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
            FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,
                                                                     interface,
                                                                     celAttr);

            // If there's no chance that the CEL expression matches any child
            // locations, stop the Op from continuing its recursion past this
            // point in the hierarchy. This isn't required, but improves
            // efficiency.
            if (!info.canMatchChildren)
            {
                interface.stopChildTraversal();
            }

            // If the CEL doesn't match the current location, stop cooking
            if (!info.matches)
            {
                return;
            }
        }

        // Get the string attributes passed in as opArgs
        FnAttribute::StringAttribute filePathAttr =
            interface.getOpArg("filepath");
        FnAttribute::StringAttribute soFilePathAttr =
            interface.getOpArg("sofilepath");
        FnAttribute::StringAttribute groupNameAttr =
            interface.getOpArg("groupname");

        // Ensure the validity of the arguments
        if (!filePathAttr.isValid())
        {
            Foundry::Katana::ReportError(interface,
                                         "Invalid \"filepath\" attribute.");
            return;
        }

        // Get the string value of the attribute
        const std::string filePath = filePathAttr.getValue();

        // If we don't explicitly want to query validity, we can request the
        // value of the attribute, and both suppress errors and provide a
        // default value for if they're not valid. getValue() takes a default
        // value (used when a failure state is encountered) and a boolean
        // indicating whether to throw an exception when the attribute is
        // invalid. By calling getValue("", false) here, we're getting the
        // value of the attribute, but if the attribute is invalid, getValue()
        // returns "", and no exception will be thrown.
        const std::string soFilePath = soFilePathAttr.getValue("", false);
        const std::string groupName = groupNameAttr.getValue("", false);

        try
        {
            // Load the attribute data from the file provided, using the
            // shared object path if needed
            const AttrData* attributeData = loadFileData(filePath, soFilePath);

            if (!attributeData)
            {
                // Make sure we actually got some data back
                return;
            }

            // getOutputName() returns the name of the scene graph location
            // that we're creating data for now, so check to see if there's a
            // match for the name in the attribute cache. getOutputName()
            // returns the leaf name for the outputLocation, e.g. if we're
            // cooking at "/root/world/geo", then "geo" will be returned.
            // To get the full location path, we could use
            // getOutputLocationPath().
            AttrData::const_iterator locationIt =
                attributeData->find(interface.getOutputName());
            if (locationIt != attributeData->end())
            {
                // There is a match, so walk over the data in the entry and
                // populate our output's attributes
                AttrDataEntries attrEntries = locationIt->second;
                AttrDataEntries::iterator attrIt;

                if (!groupName.empty())
                {
                    // Apply the attributes underneath a group attribute. The
                    // GroupBuilder is a useful class to help with this. Build
                    // the attributes on the GroupBuilder (rather than directly
                    // on the interface), and then set the builder's result on
                    // the interface under the group name the user has asked
                    // for.
                    FnAttribute::GroupBuilder gb;
                    for (attrIt = attrEntries.begin();
                         attrIt != attrEntries.end(); ++attrIt)
                    {
                        gb.set(attrIt->first.c_str(),
                               FnAttribute::StringAttribute(
                                   attrIt->second.c_str()));
                    }

                    // GroupBuilder::build() will construct a GroupAttribute
                    // that we can set on the output location using setAttr()
                    interface.setAttr(groupName, gb.build());
                }
                else
                {
                    // Apply the attributes directly on the top-level group.
                    // We don't need to use a GroupBuilder here. We directly
                    // set the attribute names/values on the output location
                    // using the interface.
                    for (attrIt = attrEntries.begin();
                         attrIt != attrEntries.end(); ++attrIt)
                    {
                        interface.setAttr(attrIt->first,
                                          FnAttribute::StringAttribute(
                                              attrIt->second.c_str()), false);
                    }
                }
            }
        }
        catch (std::runtime_error &e)
        {
            // File load failed. Report the error.
            Foundry::Katana::ReportError(interface, e.what());
            return;
        }
    }

    static void flush()
    {
        // Flush the data cache. This can be prompted by the user clicking the
        // "Flush Caches" button in the UI, or elsewhere by Katana internally.
        // Reset the data cache to an empty one.
        m_dataCache.reset(new AttrDataCache);
    }

private:

    // Loads the contents of the file with the given name using the reader
    // plug-in with the given .so filename. If no soFilename is given, the
    // KatanaAttrFileReader will be used by default.
    static const AttrData* loadFileData(const std::string &filename,
                                        const std::string &soFilename)
    {
        // Query the AttributeKeyedCache cache for the AttrData associated
        // with the given file name and .so file name. The implementation of
        // the AttrDataCache will handle loading the data from the file, if
        // it's not yet been loaded.
        AttrDataCache::IMPLPtr result = m_dataCache->getValue(
            FnAttribute::GroupBuilder()
                .set("filename", FnAttribute::StringAttribute(filename))
                .set("soFilename", FnAttribute::StringAttribute(soFilename))
                .build()
        );

        return result.get();
    }

    // Static cache mapping filenames to loaded data
    static AttrDataCache::Ptr m_dataCache;
};

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

AttrDataCache::Ptr AttributeFile_InOp::m_dataCache(new AttrDataCache());

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

DEFINE_GEOLIBOP_PLUGIN(AttributeFile_InOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(AttributeFile_InOp, "AttributeFile_In", 0, 1);
}

