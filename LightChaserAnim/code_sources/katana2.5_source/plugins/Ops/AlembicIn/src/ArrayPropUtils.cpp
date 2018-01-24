#include "ArrayPropUtils.h"
#include <FnAttribute/FnDataBuilder.h>


namespace AlembicIn
{

// read iProp.prop into oGb
template <typename attrT, typename builderT, typename podT>
void readArrayProp(
        ArrayProp & iProp,
        const OpArgs & iArgs,
        FnAttribute::GroupBuilder & oGb)
{
    size_t extent = iProp.prop.getDataType().getExtent();
    int64_t tupleSize = extent;

    builderT b(tupleSize);

    Alembic::Abc::TimeSamplingPtr ts = iProp.prop.getTimeSampling();
    SampleTimes sampleTimes;
    iArgs.getRelevantSampleTimes(ts, iProp.prop.getNumSamples(), sampleTimes);
    size_t numVals = 0;
    Alembic::Util::Dimensions dims;
    if (!sampleTimes.empty())
    {
        SampleTimes::iterator it = sampleTimes.begin();
        iProp.prop.getDimensions(dims, Alembic::Abc::ISampleSelector(*it));
        numVals = dims.numPoints();
        ++it;

        // make sure every sample we are using is the same size
        bool sameSize = true;
        for (; it != sampleTimes.end(); ++it)
        {
            iProp.prop.getDimensions(dims, Alembic::Abc::ISampleSelector(*it));
            if (numVals != dims.numPoints())
            {
                sameSize = false;
                break;
            }
        }

        // not the same, use just a single time
        if (!sameSize)
        {
            sampleTimes.clear();
            sampleTimes.push_back(iArgs.getAbcFrameTime());
            Alembic::Abc::ISampleSelector ss(*sampleTimes.begin());
            iProp.prop.getDimensions(dims, ss);
            numVals = dims.numPoints();
        }
    }

    for (SampleTimes::iterator it = sampleTimes.begin();
         it != sampleTimes.end(); ++it)
    {
        Alembic::Abc::ISampleSelector ss(*it);

        const size_t numPodValues = extent * numVals;
        std::vector<typename attrT::value_type> value;
        size_t bufOffset = 0;

        if (iProp.name == "geometry.poly.startIndex" && numVals != 0)
        {
            bufOffset = 1;
        }

        value.resize(numPodValues + bufOffset);
        try
        {
            // Perform a conversion from the stored POD type to the proper
            // POD type for the Katana attribute.
            iProp.prop.getAs(&value[bufOffset], FnAttrTypeToPODType(attrT::getKatAttributeType()), ss);
        }
        catch(Alembic::Util::Exception&)
        {
            // Because certain types cannot be automatically converted by
            // certain backends (HDF5 does not do well converting float16 to
            // float32, for example), catch the exception and perform a direct
            // cast below.  We don't do this for everything, because the
            // above is faster and works for the vast majority of cases.

            Alembic::Abc::ArraySamplePtr valuePtr;
            iProp.prop.get(valuePtr, ss);
            if (valuePtr && valuePtr->valid())
            {
                const podT* typedData = static_cast<const podT*>(valuePtr->getData());
                for (size_t i = 0; i < numPodValues; ++i)
                {
                    value[i + bufOffset] = static_cast<typename attrT::value_type> (typedData[i]);
                }
            }
        }

        if (iProp.name == "geometry.poly.startIndex" && !value.empty())
        {
            for (size_t i = 2; i < value.size(); ++i)
            {
                value[i] += value[i-1];
            }
        }

        if (sampleTimes.size() == 1)
        {
            // hopefully this will use a fancy attr
            oGb.set(iProp.name,
                attrT(&(value.front()), value.size(), tupleSize));
        }
        else
        {
            b.set(value, iArgs.getRelativeSampleTime(*it));
        }
    }

    if (sampleTimes.size() > 1)
    {
        oGb.set(iProp.name, b.build());
    }
}

void arrayPropertyToAttr(ArrayProp & iProp,
    const OpArgs & iArgs,
    FnAttribute::GroupBuilder & oGb)
{
    switch(iProp.asPod)
    {
        case Alembic::Util::kBooleanPOD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::bool_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kUint8POD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::uint8_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kInt8POD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::int8_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kUint16POD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::uint16_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kInt16POD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::int16_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kUint32POD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::uint32_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kInt32POD:
            readArrayProp<FnAttribute::IntAttribute,
                          FnAttribute::IntBuilder,
                          Alembic::Util::int32_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kFloat16POD:
            readArrayProp<FnAttribute::FloatAttribute,
                          FnAttribute::FloatBuilder,
                          Alembic::Util::float16_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kFloat32POD:
            readArrayProp<FnAttribute::FloatAttribute,
                          FnAttribute::FloatBuilder,
                          Alembic::Util::float32_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kFloat64POD:
            readArrayProp<FnAttribute::DoubleAttribute,
                          FnAttribute::DoubleBuilder,
                          Alembic::Util::float64_t>(
                iProp, iArgs, oGb);
        break;

        case Alembic::Util::kStringPOD:
            readArrayProp<FnAttribute::StringAttribute,
                          FnAttribute::StringBuilder,
                          std::string>(
                iProp, iArgs, oGb);
        break;

        default:
        break;
    }
}

void arrayPropertyToAttr(Alembic::Abc::ICompoundProperty & iParent,
    const Alembic::Abc::PropertyHeader & iPropHeader,
    const std::string & iPropName,
    FnKatAttributeType iType,
    AbcCookPtr ioCook,
    FnAttribute::GroupBuilder & oStaticGb)
{
    Alembic::Abc::IArrayProperty prop(iParent, iPropHeader.getName());

    // bad prop don't bother with it
    if (!prop.valid() || prop.getNumSamples() == 0)
    {
        return;
    }

    ArrayProp item;
    item.name = iPropName;
    item.prop = prop;
    item.asPod = iPropHeader.getDataType().getPod();

    if (!prop.isConstant() && item.asPod != Alembic::Util::kUnknownPOD)
    {
        ioCook->arrayProps.push_back(item);
        return;
    }

    OpArgs defaultArgs;
    arrayPropertyToAttr(item, defaultArgs, oStaticGb);
}

}
