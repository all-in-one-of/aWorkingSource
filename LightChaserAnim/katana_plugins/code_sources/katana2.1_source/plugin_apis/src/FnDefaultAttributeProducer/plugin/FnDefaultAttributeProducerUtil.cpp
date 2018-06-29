#include <FnDefaultAttributeProducer/plugin/FnDefaultAttributeProducerUtil.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnHintUtil.h>

#include <sstream>

FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_ENTER
{
namespace DapUtil
{

#define META "__meta"
#define METADOT "__meta."

namespace
{
    std::string _GetAttrMetaDataPath(const std::string & inputPath)
    {
        std::vector<std::string> tokens;
        pystring::split(inputPath, tokens, ".");

        return pystring::join(".c.", tokens);
    }
}

void SetAttrHints(FnAttribute::GroupBuilder & gb,
        const std::string & attrPath,
        const FnAttribute::GroupAttribute & hintsGroup)
{
    if (!hintsGroup.isValid()) return;
    gb.set(METADOT + _GetAttrMetaDataPath(attrPath) + ".hints",
            hintsGroup);
}

FnAttribute::GroupAttribute GetAttributeHints(
        const FnAttribute::GroupAttribute& defaultsGroup,
        const std::string& attributePath)
{
    return defaultsGroup.getChildByName(
            METADOT + _GetAttrMetaDataPath(attributePath) + ".hints");
}

void SetHintsForAllChildren(FnAttribute::GroupBuilder & gb,
        const std::string & attrPath,
        const FnAttribute::GroupAttribute & hintsGroup)
{
    if (!hintsGroup.isValid()) return;
    std::string attrRoot = METADOT + _GetAttrMetaDataPath(attrPath) + ".c";
    gb.set(attrRoot,  hintsGroup);
}

void CopyAttrHints(
        FnAttribute::GroupBuilder & gb,
        const FnAttribute::GroupAttribute & srcGrp,
        const std::string & srcAttrPath,
        const std::string & dstAttrPath,
        bool recursive)
{
    if (recursive)
    {
        FnAttribute::GroupAttribute srcMeta = srcGrp.getChildByName(
                METADOT + _GetAttrMetaDataPath(srcAttrPath));

        if (srcMeta.isValid())
        {
            gb.set(METADOT + _GetAttrMetaDataPath(dstAttrPath),
                    srcMeta);
        }

    }
    else
    {
        FnAttribute::GroupAttribute srcHints = srcGrp.getChildByName(
                METADOT + _GetAttrMetaDataPath(srcAttrPath + ".hints"));

        if (srcHints.isValid())
        {
            SetAttrHints(gb, dstAttrPath, srcHints);
        }
    }
}

void PromoteAttrHints(FnAttribute::GroupBuilder & gb,
        const FnAttribute::GroupAttribute & attrWithHints,
        const std::string & newAttrPath)
{
    if (!attrWithHints.isValid()) return;

    FnAttribute::GroupAttribute metaAttr = attrWithHints.getChildByName(META);
    if (!metaAttr.isValid()) return;

    std::string newMetaPath =
            METADOT + _GetAttrMetaDataPath(newAttrPath) + ".c";
    gb.set(newMetaPath, metaAttr);
}

FnAttribute::GroupAttribute StripAttrHints(
        const FnAttribute::GroupAttribute & attr)
{
    FnAttribute::GroupBuilder gb;
    gb.update(attr);
    gb.del(META);
    return gb.build();
}



void SetContainerHints(
        FnAttribute::GroupBuilder & gb,
        const std::string & attrPath,
        const std::string & containerPath,
        const FnAttribute::GroupAttribute & hintsGroup)
{
    std::ostringstream buffer;
    buffer << METADOT;
    buffer << _GetAttrMetaDataPath(attrPath);
    buffer << ".containerHints.";
    buffer << _GetAttrMetaDataPath(containerPath);
    buffer << ".hints";

    gb.set(buffer.str(), hintsGroup);
}

FnAttribute::GroupAttribute GetContainerHints(
        const FnAttribute::GroupAttribute& defaultsGroup,
        const std::string& attributePath, const std::string& containerPath)
{

    std::ostringstream buffer;
    buffer << METADOT;
    buffer << _GetAttrMetaDataPath(attributePath);
    buffer << ".containerHints.";
    buffer << _GetAttrMetaDataPath(containerPath);
    buffer << ".hints";

    return defaultsGroup.getChildByName(buffer.str());
}

void ParseAttributeHints(
        const std::string & argsRoot,
        FnAttribute::GroupBuilder & mainGb,
        FnAttribute::GroupBuilder & argsGb,
        FnAttribute::GroupAttribute & groupAttr,
        const std::string & attrPath)
{
    for (int64_t i = 0, e = groupAttr.getNumberOfChildren();
            i != e; ++i)
    {
        std::string childName = groupAttr.getChildName(i);

        if (pystring::endswith(childName, "__hints"))
        {
            std::string childPath = attrPath.empty() ?
                    childName : attrPath + "." + childName;

            argsGb.del(childPath);


            FnAttribute::GroupAttribute hintAttr =
                    FnGeolibServices::FnHintUtil::getHintGroup(
                            groupAttr.getChildByIndex(i));

            if (hintAttr.isValid())
            {
                SetAttrHints(mainGb,
                        argsRoot + "." + pystring::slice(
                        childPath, 0, -7), hintAttr);
            }
        }
        else
        {
            FnAttribute::GroupAttribute childGroup =
                    groupAttr.getChildByIndex(i);

            if (childGroup.isValid())
            {
                ParseAttributeHints(argsRoot,
                            mainGb,
                            argsGb,
                            childGroup,
                            attrPath.empty() ? childName : attrPath + "." + childName);
            }
        }
    }
}


} // namespace DapUtil
}
FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_EXIT
