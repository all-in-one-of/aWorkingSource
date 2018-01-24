#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/util/Path.h>

#include <pystring/pystring.h>

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include <sstream>


namespace { //anonymous

class AttributeCopyOp : public Foundry::Katana::GeolibOp
{
public:

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
                Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }
    
    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        const int inputIndex = int(FnAttribute::FloatAttribute(
                interface.getOpArg("inputIndex")).getValue(1.0, false));
        if (interface.getNumInputs() < 1 + inputIndex)
        {
            std::ostringstream os;
            os << "AttributeCopy Op requires at least " << 1 + inputIndex
               << " connected inputs, as inputIndex is " << inputIndex << ".";
            Foundry::Katana::ReportError(interface, os.str().c_str());
            interface.stopChildTraversal();
            return;
        }

        std::string toRoot = FnAttribute::StringAttribute(
                interface.getOpArg("toRoot")).getValue("", false);

        std::string locationPath = interface.getOutputLocationPath();

        FnAttribute::StringAttribute celAttr = interface.getOpArg("toCEL");
        if (celAttr.isValid() && !celAttr.getValue().empty())
        {
            FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
            FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,
                                                                     interface,
                                                                     celAttr);

            if (!info.canMatchChildren)
            {
                interface.stopChildTraversal();
            }
            if (!info.matches)
            {
                return;
            }
        }

        if (toRoot.empty() ||
            (!FnGeolibUtil::Path::IsAncestorOrEqual(locationPath, toRoot) &&
             !FnGeolibUtil::Path::IsAncestor(toRoot, locationPath)))
        {
            // No root provided, or we're not in the same location tree
            interface.stopChildTraversal();
            return;
        }

        if (!FnGeolibUtil::Path::IsAncestorOrEqual(toRoot, locationPath))
        {
            // We haven't reached our root location yet, so stop cooking,
            // but don't stop child traversal
            return;
        }

        const std::string relativePath =
            FnGeolibUtil::Path::NormalizedRelativePath(toRoot, locationPath);

        std::string fromRoot = FnAttribute::StringAttribute(
                interface.getOpArg("fromRoot")).getValue("", false);

        FnAttribute::GroupAttribute entries = interface.getOpArg("entries");

        if (!entries.isValid() || entries.getNumberOfChildren() == 0)
        {
            interface.stopChildTraversal();
            return;
        }


        std::string fromLocationPath = fromRoot;
        if (!relativePath.empty())
        {
            fromLocationPath = FnGeolibUtil::Path::Join(fromLocationPath,
                                                        relativePath);
        }

        // Ensure that the hierarchy matches between source/target
        if (!interface.doesLocationExist(fromLocationPath, inputIndex))
        {
            std::ostringstream os;
            os << "Mismatched hierarchy detected in AttributeCopy. Source "
               << "scenegraph location (input index " << inputIndex << ") "
               << "cannot be found: " << fromLocationPath;

            Foundry::Katana::ReportError(interface, os.str());
            return;
        }

        for (int64_t i = 0, e = entries.getNumberOfChildren(); i < e; ++i)
        {
            FnAttribute::GroupAttribute entry = entries.getChildByIndex(i);
            if (!entry.isValid())
            {
                continue;
            }

            std::string fromAttrName = FnAttribute::StringAttribute(
                    entry.getChildByName("fromAttr")).getValue("", false);
            std::string toAttrName = FnAttribute::StringAttribute(
                    entry.getChildByName("toAttr")).getValue("", false);

            if (fromAttrName.empty() || toAttrName.empty())
            {
                continue;
            }

            FnAttribute::Attribute attr = interface.getAttr(
                    fromAttrName, fromLocationPath, inputIndex);
            if (attr.isValid())
            {
                // Groups that are *implicitly created* during the copy are
                // always groupInherit=true, regardless of the status of the
                // source attribute and any containing group attributes.
                interface.setAttr(toAttrName, attr);
            }
        }
    }
};

DEFINE_GEOLIBOP_PLUGIN(AttributeCopyOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(AttributeCopyOp, "AttributeCopy", 0, 1);
}
