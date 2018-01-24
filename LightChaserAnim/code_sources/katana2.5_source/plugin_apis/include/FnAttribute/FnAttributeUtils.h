#ifndef FoundryKatanaAttributeUtils_H
#define FoundryKatanaAttributeUtils_H

#include <vector>
#include <set>
#include <map>
#include <ostream>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnAttributeAPI.h>
#include <FnAttribute/FnGroupBuilder.h>

FNATTRIBUTE_NAMESPACE_ENTER
{
    ////////////////////////////////////////////////////////////////////////////
    //
    // WARNING - these are likely to be moved to an alternate library / header
    //

    // To compute a global attribute, start at leaf, and walk up, querying
    // the specified attribute.
    //
    // If, at any point, a non-group attr is encountered, return that.
    // If, at any point, an enclosing group attr is non-inheriting, stop walking up.
    //
    // Finally, collapse the accumulated GroupAttrs into a single return value.
    //
    // NOTE: localAttributes vector is ordered from LEAF to ROOT
    // index[0] = LEAF
    // index[n-1] = ROOT

    FNATTRIBUTE_API
    Attribute GetGlobalAttribute(
        const std::vector<GroupAttribute> & localAttributes);

    // return a1.extend(a2)
    // Similar to python's "extend" array function, though this does type
    // checking. If types a1 and a2 do not match, an invalid attr is
    // returned.

    FNATTRIBUTE_API
    Attribute ExtendDataAttr(const Attribute & a1,
                             const Attribute & a2);

    // Given an attribute, add all the available time samples to the
    // incoming set. GroupAttributes are deeply recursed, with all DataAttribute
    // leaves added.

    FNATTRIBUTE_API
    void GetAttributeTimeSamples(std::set<float> * samples, const Attribute & attr);

    // Given an attribute, if it is multi-sampled and the values are the same,
    // return a new attribute with only the initial time-sample. If the values
    // are varying, the original attribute will be returned unmodified.
    // GroupAttributes will be traversed, with this applied to all leaves
    FNATTRIBUTE_API
    Attribute RemoveTimeSamplesIfAllSame(const Attribute & attr);

    // Given a shutter range, this will remove all unnecessary time samples.
    // Note that to maintain interpolation accuraccy, if a timesample doesnt
    // fall exactly on shutterOpen, shutterClose, one extra sample outside the
    // specified range will be kept. (But samples further outside will be
    // removed).

    FNATTRIBUTE_API
    Attribute RemoveTimeSamplesUnneededForShutter(const Attribute & attr,
        float shutterOpen, float shutterClose);

    // Resample the specified value to match the time samples present
    // in the timeReference. New values will be created using fillInterpSample
    // for FloatAttribute and DoubleAttributes, and getNearestSample
    // for IntAttributes, StringAttribute.
    // NullAttributes are returned 'as is'
    // GroupAttributes are recursed, and converted at all appropriate leaves.
    //
    // Note: the attr values for timeReference are ignored.

    FNATTRIBUTE_API
    FnAttribute::Attribute ResampleAttrWithInterp(
        const FnAttribute::Attribute & value,
        const FnAttribute::DataAttribute & timeReference);

    // Useful for pretty printing of FnAttributes
    FNATTRIBUTE_API
    void GetAttrValueAsPrettyText(std::ostream & os,
        const Attribute & attr,
        int maxArrayValues=-1);

    FNATTRIBUTE_API
    void GetAttrTypeAsPrettyText(std::ostream& os, const Attribute& attr);
}
FNATTRIBUTE_NAMESPACE_EXIT

#endif // FoundryKatanaAttributeUtils_H
