#ifndef INCLUDED_FNDEFAULTATTRIBUTEPRODUCERUTIL_H
#define INCLUDED_FNDEFAULTATTRIBUTEPRODUCERUTIL_H

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include "ns.h"

FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_ENTER
{
namespace DapUtil
{

    void SetAttrHints(
            FnAttribute::GroupBuilder & gb,
            const std::string & attrPath,
            const FnAttribute::GroupAttribute & hintsGroup);

    void SetHintsForAllChildren(
            FnAttribute::GroupBuilder & gb,
            const std::string & attrPath,
            const FnAttribute::GroupAttribute & hintsGroup);

    void CopyAttrHints(
            FnAttribute::GroupBuilder & gb,
            const FnAttribute::GroupAttribute & srcGrp,
            const std::string & srcAttrPath,
            const std::string & dstAttrPath,
            bool recursive);


    void PromoteAttrHints(
            FnAttribute::GroupBuilder & gb,
            const FnAttribute::GroupAttribute & attrWithHints,
            const std::string & attrPath);

    FnAttribute::GroupAttribute StripAttrHints(
            const FnAttribute::GroupAttribute & attr);


    void SetContainerHints(
            FnAttribute::GroupBuilder & gb,
            const std::string & attrPath,
            const std::string & containerPath,
            const FnAttribute::GroupAttribute & hintsGroup);

    /// For the given Defaults Group, returns for the attribute at
    /// |attributePath|, the container hints for the container specified by
    /// |containerPath|.
    ///
    /// @param defaultsGroup: The defaults group being cooked by a DAP.
    /// @param attributePath: The attribute whose container hints are to be
    ///     returned.
    /// @param containerPath: The container whose hints are to be returned.
    FnAttribute::GroupAttribute GetContainerHints(
            const FnAttribute::GroupAttribute& defaultsGroup,
            const std::string& attributePath, const std::string& containerPath);

    /// For the given Defaults Group, returns the parameter-level hints for the
    /// attribute at |attributePath|.
    ///
    /// @param defaultsGroup: The Defaults Group being cooked by a DAP.
    /// @param attributePath: The attribute whose hints are to be returned.
    FnAttribute::GroupAttribute GetAttributeHints(
            const FnAttribute::GroupAttribute& defaultsGroup,
            const std::string& attributePath);

    void ParseAttributeHints(
            const std::string & argsRoot,
            FnAttribute::GroupBuilder & mainGb,
            FnAttribute::GroupBuilder & argsGb,
            FnAttribute::GroupAttribute & groupAttr,
            const std::string & attrPath);

} // namespace DapUtil
}
FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_EXIT

#endif // INCLUDED_FNDEFAULTATTRIBUTEPRODUCERUTIL_H

