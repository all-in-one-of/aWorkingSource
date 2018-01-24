#include <FnAttribute/FnAttributeUtils.h>
#include <FnAttribute/FnDataBuilder.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <limits>
#include <iterator>

#include <pystring/pystring.h>

FNATTRIBUTE_NAMESPACE_ENTER
{
    Attribute GetGlobalAttribute(const std::vector<GroupAttribute> & groups)
    {
        if(groups.empty())
        {
            return Attribute();
        }
        else if(groups.size() == 1)
        {
            return groups[0];
        }
        else
        {
            GroupBuilder gb;
            for(std::vector<GroupAttribute>::const_reverse_iterator riter = groups.rbegin(),
                rend = groups.rend(); riter != rend; ++riter)
            {
                if(!riter->isValid()) continue;
                gb.deepUpdate(*riter);
            }

            return gb.build();
        }
    }

    namespace
    {
        template <typename T>
        DataAttribute _ExtendDataAttrT(const T & a1,
                                                    const T & a2)
        {
            std::set<float> alltimes;
            int64_t numSampleTimes = a1.getNumberOfTimeSamples();
            for (int64_t i = 0; i < numSampleTimes; ++i)
            {
                alltimes.insert(a1.getSampleTime(i));
            }

            numSampleTimes = a2.getNumberOfTimeSamples();
            for (int64_t i = 0; i < numSampleTimes; ++i)
            {
                alltimes.insert(a2.getSampleTime(i));
            }

            DataBuilder<T> databuilder(a1.getTupleSize());
            std::vector<typename T::value_type> outputvec;

            for(std::set<float>::iterator iter = alltimes.begin(),
                    end = alltimes.end(); iter != end; ++iter)
            {
                float sampleTime = *iter;

                typename T::array_type v1 = a1.getNearestSample(sampleTime);
                typename T::array_type v2 = a2.getNearestSample(sampleTime);

                // TODO: Use resize and std::copy (no back_inserter) instead?
                outputvec.clear();
                outputvec.reserve(v1.size()+v2.size());
                std::copy(v1.begin(), v1.end(), std::back_inserter(outputvec));
                std::copy(v2.begin(), v2.end(), std::back_inserter(outputvec));

                databuilder.set(outputvec, sampleTime);
            }

            return databuilder.build();
        }
    }

    Attribute ExtendDataAttr(const Attribute & a1,
                             const Attribute & a2)
    {
        if(a1.getType() != a2.getType())
        {
            return Attribute();
        }

        if(a1.getType()==kFnKatAttributeTypeInt)
        {
            return _ExtendDataAttrT<IntAttribute>(a1,a2);
        }
        else if(a1.getType()==kFnKatAttributeTypeFloat)
        {
            return _ExtendDataAttrT<FloatAttribute>(a1,a2);
        }
        else if(a1.getType()==kFnKatAttributeTypeDouble)
        {
            return _ExtendDataAttrT<DoubleAttribute>(a1,a2);
        }
        else if(a1.getType()==kFnKatAttributeTypeString)
        {
            return _ExtendDataAttrT<StringAttribute>(a1,a2);
        }

        return Attribute();
    }

    void GetAttributeTimeSamples(std::set<float> * samples, const Attribute & attr)
    {
        if (!samples || !attr.isValid()) return;

        FnKatAttributeType type = attr.getType();
        switch (type)
        {
            case kFnKatAttributeTypeInt:
            case kFnKatAttributeTypeFloat:
            case kFnKatAttributeTypeDouble:
            case kFnKatAttributeTypeString:
            {
                DataAttribute a = attr;
                for (int64_t t = 0, e = a.getNumberOfTimeSamples(); t<e; ++t)
                {
                    samples->insert(a.getSampleTime(t));
                }
                break;
            }
            case kFnKatAttributeTypeGroup:
            {
                GroupAttribute g = attr;
                for (int64_t i=0, e = g.getNumberOfChildren(); i<e; ++i)
                {
                    GetAttributeTimeSamples(samples, g.getChildByIndex(i));
                }
                break;
            }
        }
    }

    Attribute ResampleAttrWithInterp(
        const Attribute & value,
        const DataAttribute & timeAttr)
    {
        if (!timeAttr.isValid()) return Attribute();

        int64_t ref_times = timeAttr.getNumberOfTimeSamples();

        FnKatAttributeType type = value.getType();
        switch (type)
        {
            case kFnKatAttributeTypeNull:
                return value;
            case kFnKatAttributeTypeInt:
            {
                IntAttribute intval = value;

                int64_t src_times = intval.getNumberOfTimeSamples();
                if (src_times == ref_times) {
                    if (src_times == 1)
                       return value; // nothing to re-sample
                    bool all_equal = true;
                    for (int64_t t = 0; all_equal && t < ref_times; ++t)
                       all_equal = timeAttr.getSampleTime(t) == intval.getSampleTime(t);
                    if (all_equal)
                       return value; // nothing to re-sample
                }

                IntBuilder builder(intval.getTupleSize());
                for (int64_t t = 0; t < ref_times; ++t)
                {
                    std::vector<int32_t> & data = builder.get(timeAttr.getSampleTime(t));
                    IntAttribute::array_type sample =
                        intval.getNearestSample(timeAttr.getSampleTime(t));
                    if (!sample.empty()) data.assign(sample.begin(), sample.end());
                }
                return builder.build();
            }
            case kFnKatAttributeTypeFloat:
            {
                FloatAttribute floatval = value;

                int64_t src_times = floatval.getNumberOfTimeSamples();
                if (src_times == ref_times) {
                    if (src_times == 1)
                       return value; // nothing to re-sample
                    bool all_equal = true;
                    for (int64_t t = 0; all_equal && t < ref_times; ++t)
                       all_equal = timeAttr.getSampleTime(t) == floatval.getSampleTime(t);
                    if (all_equal)
                       return value; // nothing to re-sample
                }

                FloatBuilder builder(floatval.getTupleSize());
                for (int64_t t = 0; t < ref_times; ++t)
                {
                    std::vector<float> & data = builder.get(timeAttr.getSampleTime(t));
                    data.resize(floatval.getNumberOfValues());
                    if (data.size() > 0)
                    {
                        floatval.fillInterpSample(data.data(), data.size(),
                            timeAttr.getSampleTime(t));
                    }
                }
                return builder.build();
            }
            case kFnKatAttributeTypeDouble:
            {
                DoubleAttribute doubleval = value;

                int64_t src_times = doubleval.getNumberOfTimeSamples();
                if (src_times == ref_times) {
                    if (src_times == 1)
                       return value; // nothing to re-sample
                    bool all_equal = true;
                    for (int64_t t = 0; all_equal && t < ref_times; ++t)
                       all_equal = timeAttr.getSampleTime(t) == doubleval.getSampleTime(t);
                    if (all_equal)
                       return value; // nothing to re-sample
                }

                DoubleBuilder builder(doubleval.getTupleSize());
                for (int64_t t = 0; t < ref_times; ++t)
                {
                    std::vector<double> & data = builder.get(timeAttr.getSampleTime(t));
                    data.resize(doubleval.getNumberOfValues());
                    if (data.size() > 0)
                    {
                        doubleval.fillInterpSample(data.data(), data.size(),
                            timeAttr.getSampleTime(t));
                    }
                }
                return builder.build();
            }
            case kFnKatAttributeTypeString:
            {
                StringAttribute stringval = value;

                int64_t src_times = stringval.getNumberOfTimeSamples();
                if (src_times == ref_times) {
                    if (src_times == 1)
                       return value; // nothing to re-sample
                    bool all_equal = true;
                    for (int64_t t = 0; all_equal && t < ref_times; ++t)
                       all_equal = timeAttr.getSampleTime(t) == stringval.getSampleTime(t);
                    if (all_equal)
                       return value; // nothing to re-sample
                }

                StringBuilder builder(stringval.getTupleSize());
                for (int64_t t = 0; t < ref_times; ++t)
                {
                    std::vector<std::string> & data = builder.get(timeAttr.getSampleTime(t));
                    StringAttribute::array_type sample =
                        stringval.getNearestSample(timeAttr.getSampleTime(t));
                    if (!sample.empty()) data.assign(sample.begin(), sample.end());
                }
                return builder.build();
            }
            case kFnKatAttributeTypeGroup:
            {
                GroupAttribute groupval = value;
                GroupAttribute::NamedAttrVector_Type children;
                groupval.fillChildVector(&children);
                for (size_t i=0; i<children.size(); ++i)
                {
                    children[i].second = ResampleAttrWithInterp(children[i].second, timeAttr);
                }
                return GroupAttribute(children, groupval.getGroupInherit());
            }
        }

        return Attribute();
    }

    // Given an attribute, if it is multi-sampled and the values are the same,
    // return a new attribute with only the initial time-sample. If the values
    // are varying, the original attribute will be returned unmodified.
    // GroupAttributes will be traversed, with this applied to all leaves

    namespace
    {
        template <typename T>
        Attribute _RemoveTimeSamplesIfAllSame(const T & attr)
        {
            int64_t nt = attr.getNumberOfTimeSamples();
            if (nt <= 1) return attr;

            typename T::array_type v0 = attr.getNearestSample(attr.getSampleTime(0));
            if (v0.empty()) return attr;

            bool all_same = true;
            for (int64_t i = 1; all_same && (i < nt); i++)
            {
                typename T::array_type v1 = attr.getNearestSample(attr.getSampleTime(i));
                all_same &= (memcmp(v0.data(), v1.data(), v0.size() * sizeof(typename T::value_type)) == 0);
            }

            if (all_same)
            {
                return T(v0.data(), attr.getNumberOfValues(), attr.getTupleSize());
            }
            else
            {
                return attr;
            }
        }
        
        template <>
        Attribute _RemoveTimeSamplesIfAllSame(const StringAttribute & attr)
        {
            int64_t nt = attr.getNumberOfTimeSamples();
            if (nt <= 1) return attr;

            StringAttribute::array_type v0 = attr.getNearestSample(attr.getSampleTime(0));
            if (v0.empty()) return attr;

            size_t numelements = v0.size();

            bool all_same = true;
            for (int64_t i = 1; all_same && (i < nt); i++)
            {
                StringAttribute::array_type v1 = attr.getNearestSample(attr.getSampleTime(i));
                for(size_t e=0; all_same && (e < numelements); e++)
                {
                    all_same &= (strcmp(v0.data()[e], v1.data()[e]) == 0);
                }
            }

            if (all_same)
            {
                // This isn't ideal to go through a vector<std::string>
                // intermediary, but due to a const mismatch between
                // getNearestSample and the StringAttribute constructor
                // it's simplest to do this. (Not to mention, multi-sampled
                // StringAttribute(s) are exceedingly rare in practice).
                return StringAttribute(
                    std::vector<std::string>(v0.begin(), v0.end()),
                        attr.getTupleSize());
            }
            else
            {
                return attr;
            }
        }
    }

    Attribute RemoveTimeSamplesIfAllSame(const Attribute & attr)
    {
        if (!attr.isValid()) return Attribute();

        FnKatAttributeType type = attr.getType();
        switch (type)
        {
            case kFnKatAttributeTypeGroup:
            {
                GroupAttribute groupval = attr;
                GroupAttribute::NamedAttrVector_Type children;
                groupval.fillChildVector(&children);
                for (size_t i=0; i<children.size(); ++i)
                {
                    children[i].second = RemoveTimeSamplesIfAllSame(children[i].second);
                }
                return GroupAttribute(children, groupval.getGroupInherit());
            }
            case kFnKatAttributeTypeNull:
                return attr;
            case kFnKatAttributeTypeInt:
                return _RemoveTimeSamplesIfAllSame<IntAttribute>(attr);
            case kFnKatAttributeTypeFloat:
                return _RemoveTimeSamplesIfAllSame<FloatAttribute>(attr);
            case kFnKatAttributeTypeDouble:
                return _RemoveTimeSamplesIfAllSame<DoubleAttribute>(attr);
            case kFnKatAttributeTypeString:
                return _RemoveTimeSamplesIfAllSame<StringAttribute>(attr);
        }

        return Attribute();
    }

    namespace
    {
    void _FindSampleTimesRelevantToShutterRange(
            const std::vector<float> & inputSamples,
            float shutterOpen, float shutterClose,
            std::vector<float> & outputSamples)
    {
        outputSamples.clear();
        outputSamples.reserve(inputSamples.size());
        
        float lastValue = -std::numeric_limits<float>::max();
        for (std::vector<float>::const_iterator I = inputSamples.begin();
            I != inputSamples.end(); ++I)
        {
            const float & value = (*I);
            if (value < shutterOpen)
            {
                std::vector<float>::const_iterator II = I;
                ++II;
                if (II != inputSamples.end() && (*II) <= shutterOpen)
                {
                    continue;
                }
            }

            if (value > shutterClose && lastValue >= shutterClose) break;
            outputSamples.push_back(value);
            lastValue = value;
        }
    }

    template <typename T>
    Attribute _RemoveTimeSamplesUnneededForShutter(const T & attr,
        float shutterOpen, float shutterClose)
    {
        int64_t nt = attr.getNumberOfTimeSamples();
        if (nt <= 1) return attr;

        std::vector<float> inputSamples(nt);
        for (int64_t i=0; i<nt; ++i) inputSamples[i] = attr.getSampleTime(i);

        std::vector<float> outputSamples;
        _FindSampleTimesRelevantToShutterRange(inputSamples,
            shutterOpen, shutterClose, outputSamples);
        if (inputSamples.size() == outputSamples.size()) return attr;

        DataBuilder<T> db(attr.getTupleSize());
        for(size_t i=0, e=outputSamples.size(); i<e; ++i)
        {
            typename T::array_type data = attr.getNearestSample(outputSamples[i]);
            db.get(outputSamples[i]).assign(data.begin(), data.end());
        }
        return db.build();
    }

    } // anon namespace
    
    Attribute RemoveTimeSamplesUnneededForShutter(const Attribute & attr,
        float shutterOpen, float shutterClose)
    {
        if (!attr.isValid()) return Attribute();
        if (shutterOpen == -std::numeric_limits<float>::max() &&
            shutterClose == std::numeric_limits<float>::max()) return attr;

        FnKatAttributeType type = attr.getType();
        switch (type)
        {
            case kFnKatAttributeTypeGroup:
            {
                GroupAttribute groupval = attr;
                GroupAttribute::NamedAttrVector_Type children;
                groupval.fillChildVector(&children);
                for (size_t i=0; i<children.size(); ++i)
                {
                    children[i].second = RemoveTimeSamplesUnneededForShutter(
                        children[i].second, shutterOpen, shutterClose);
                }
                return GroupAttribute(children, groupval.getGroupInherit());
            }
            case kFnKatAttributeTypeNull:
                return attr;
            case kFnKatAttributeTypeInt:
                return _RemoveTimeSamplesUnneededForShutter<IntAttribute>(attr, shutterOpen, shutterClose);
            case kFnKatAttributeTypeFloat:
                return _RemoveTimeSamplesUnneededForShutter<FloatAttribute>(attr, shutterOpen, shutterClose);
            case kFnKatAttributeTypeDouble:
                return _RemoveTimeSamplesUnneededForShutter<DoubleAttribute>(attr, shutterOpen, shutterClose);
            case kFnKatAttributeTypeString:
                return _RemoveTimeSamplesUnneededForShutter<StringAttribute>(attr, shutterOpen, shutterClose);
        }

        return Attribute();
    }

    ////////////////////////////////////////////////////////////////////////////

    namespace // anonymous
    {
        template<typename T> void printAttrValue_t(std::ostream & os, const T & attr,
            int maxArrayValues)
        {
            int64_t numSamples = attr.getNumberOfTimeSamples();

            if (numSamples == 1 && attr.getSampleTime(0) ==0.0f)
            {
                os << "'";
                int count = 0;
                const typename T::array_type & values = attr.getNearestSample(0.0f);
                for (typename T::array_type::const_iterator
                    iter=values.begin(); iter!=values.end(); ++iter, ++count)
                {
                    if (iter!=values.begin()) os << ",";
                    os << *iter;

                    if(maxArrayValues>0 && count>maxArrayValues)
                    {
                        os << " ...";
                        break;
                    }
                }
                os << "'";
            }
            else
            {
                os << "{";
                for (int64_t sampleIndex=0; sampleIndex<numSamples; ++sampleIndex)
                {
                    if (sampleIndex != 0) os << ", ";
                    float sampleTime = attr.getSampleTime(sampleIndex);

                    os << sampleTime << ":'";
                    int count = 0;
                    const typename T::array_type & values = attr.getNearestSample(sampleTime);
                    for (typename T::array_type::const_iterator
                        iter=values.begin(); iter!=values.end();
                        ++iter, ++count)
                    {
                        if (iter!=values.begin()) os << ",";
                        os << *iter;
                        if(maxArrayValues>0 && count>maxArrayValues)
                        {
                            os << " ...";
                            break;
                        }
                    }
                    os << "'";
                }
                os << "}";
            }
        }

        void printAttrValue(std::ostream & os,
            const Attribute & attr,
            int maxArrayValues)
        {
            switch (attr.getType())
            {
                case kFnKatAttributeTypeNull:
                {
                    os << "Null";
                }
                break;

                case kFnKatAttributeTypeInt:
                {
                    printAttrValue_t(os, IntAttribute(attr),
                        maxArrayValues);
                }
                break;

                case kFnKatAttributeTypeFloat:
                {
                    os << std::setprecision(12);
                    printAttrValue_t(os, FloatAttribute(attr),
                        maxArrayValues);
                }
                break;

                case kFnKatAttributeTypeDouble:
                {
                    os << std::setprecision(12);
                    printAttrValue_t(os, DoubleAttribute(attr),
                        maxArrayValues);
                }
                break;

                case kFnKatAttributeTypeString:
                {
                    printAttrValue_t(os, StringAttribute(attr),
                        maxArrayValues);
                }
                break;

                case kFnKatAttributeTypeGroup:
                {
                    os << "{";
                    GroupAttribute g(attr);
                    if (g.isValid())
                    {
                        if(!g.getGroupInherit()) os << "inherit=False ";
                        for(int childIndex=0; childIndex<g.getNumberOfChildren(); ++childIndex)
                        {
                            if (childIndex > 0) os << ", ";
                            os << g.getChildName(childIndex) << ":";
                            printAttrValue(os, g.getChildByIndex(childIndex), maxArrayValues);
                        }
                    }
                    os << "}";
                }
                break;

                default:
                {
                    os << "<unknown type>";
                }
                break;
            }
        }
    } // namespace

    void GetAttrValueAsPrettyText(std::ostream & os,
        const Attribute & attr,
        int maxArrayValues)
    {
        GroupAttribute groupAttr(attr);
        if(groupAttr.isValid())
        {
            os << "{";
            if(!groupAttr.getGroupInherit()) os << "inherit=False ";

            for(int childIndex=0;
                childIndex<groupAttr.getNumberOfChildren(); ++childIndex)
            {
                if (childIndex > 0) os << " ";
                os << groupAttr.getChildName(childIndex) << "=";

                printAttrValue(os, groupAttr.getChildByIndex(childIndex), maxArrayValues);
            }
            os << "}";
        }
        else
        {
            printAttrValue(os, attr, maxArrayValues);
        }
    }

    void GetAttrTypeAsPrettyText(std::ostream& os, const Attribute& attr)
    {
        if (!attr.isValid())
        {
            os << "<invalid attribute>";
            return;
        }

        switch (attr.getType())
        {
            case kFnKatAttributeTypeNull:
                os << "Null";
                break;
            case kFnKatAttributeTypeInt:
                os << "Int";
                break;
            case kFnKatAttributeTypeFloat:
                os << "Float";
                break;
            case kFnKatAttributeTypeDouble:
                os << "Double";
                break;
            case kFnKatAttributeTypeString:
                os << "String";
                break;
            case kFnKatAttributeTypeGroup:
                os << "Group";
                break;
            default:
                os << "<unknown type>";
                break;
        }
    }

}
FNATTRIBUTE_NAMESPACE_EXIT
