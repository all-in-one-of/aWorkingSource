// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>

#include <pystring/pystring.h>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <FnGeolibServices/FnArbitraryOutputAttr.h>
namespace FnGeolibServices = FNGEOLIBSERVICES_NAMESPACE;

#include <set>
#include <sstream>
#include <ErrorReporting.h>
#include <WriteRI_Util.h>

#include <FnAttribute/FnDataBuilder.h>

using namespace FnKat;

namespace PRManProcedural
{

template <typename T>
Attribute _createConstantAttrByIndex(T attr, int index)
{
    unsigned int tupleSize = attr.getTupleSize();
    std::map<float, std::vector<typename T::value_type> > inDataMap;
    typename FnKat::DataBuilder<T> outDataBuilder(tupleSize);

    int64_t numSampleTimes = attr.getNumberOfTimeSamples();
    for (int64_t i = 0; i < numSampleTimes; ++i)
    {
        float sampletime = attr.getSampleTime(i);
        const typename T::array_type & inElement = attr.getNearestSample(sampletime);

        // add each item four times, for prman varying/vertex case
        for (unsigned int count=0; count<4; ++count)
        {
            for (unsigned int i=0; i<tupleSize; ++i)
            {
                outDataBuilder.push_back(inElement[index*tupleSize+i], sampletime);
            }
        }
    }

    return outDataBuilder.build();
}

Attribute createConstantAttrByIndex(DataAttribute attr, int index)
{
    switch (attr.getType())
    {
    case kFnKatAttributeTypeFloat:
        return _createConstantAttrByIndex(FloatAttribute(attr), index);
    case kFnKatAttributeTypeInt:
        return _createConstantAttrByIndex(IntAttribute(attr), index);
    case kFnKatAttributeTypeDouble:
        return _createConstantAttrByIndex(DoubleAttribute(attr), index);
    case kFnKatAttributeTypeString:
        return _createConstantAttrByIndex(StringAttribute(attr), index);
    default:
        return Attribute();
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// BuildPolymeshData
//
// Common data shared by polymesh and subdiv surfaces.
bool BuildPolymeshData(FnScenegraphIterator sgIterator,
    RtInt *num_polys, std::vector<RtInt> *num_vertices, std::vector<RtInt> *vertices,
    AttrList * attrList)
{
    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        // No geometry group?  ugh.
        Report_Error(sgIterator.getType() + " missing geometry attribute", sgIterator);
        return false;
    }

    FnAttribute::GroupAttribute polyGroupAttr = geometryGroupAttr.getChildByName("poly");
    FnAttribute::GroupAttribute pointGroupAttr = geometryGroupAttr.getChildByName("point");
    FnAttribute::GroupAttribute vertexGroupAttr = geometryGroupAttr.getChildByName("vertex");
    if (!polyGroupAttr.isValid() || !pointGroupAttr.isValid())
    {
        // No point and poly group?  ugh.
        Report_Error(sgIterator.getType() + " missing either point or poly attribute.", sgIterator);
        return false;
    }

    FnAttribute::FloatAttribute pointAttr = pointGroupAttr.getChildByName("P");
    if (!pointAttr.isValid())
    {
        Report_Error(sgIterator.getType() + " missing point.P attribute.", sgIterator);
        return false;
    }

    FnAttribute::IntAttribute polyStartIndexAttr = polyGroupAttr.getChildByName("startIndex");
    FnAttribute::IntAttribute vertexListAttr = polyGroupAttr.getChildByName("vertexList");
    if (!polyStartIndexAttr.isValid() || !vertexListAttr.isValid())
    {
        Report_Error(sgIterator.getType() + " missing either poly.startIndex or poly.vertexList attribute.", sgIterator);
        return false;
    }

    *num_polys = polyStartIndexAttr.getNumberOfTuples();
    *num_vertices = std::vector<RtInt>(*num_polys, 0); // num_polys elements, initialized to 0.
    *vertices =  std::vector<RtInt>(vertexListAttr.getNumberOfValues(), 0);

    FnAttribute::IntConstVector polyStartIndexAttrValue = polyStartIndexAttr.getNearestSample(0);
    unsigned int srcIndex = 0;
    for(std::vector<RtInt>::size_type i = 0; srcIndex < polyStartIndexAttr.getNumberOfTuples();
        ++i, ++srcIndex)
    {
        if (srcIndex + 1 < polyStartIndexAttr.getNumberOfTuples())
            (*num_vertices)[i] = polyStartIndexAttrValue[srcIndex+1] -
                polyStartIndexAttrValue[srcIndex];
        else
            (*num_vertices)[i] = vertexListAttr.getNumberOfTuples() -
                polyStartIndexAttrValue[srcIndex];

        // Shockingly, this is all that's needed to drop empty polygons.
        if ((*num_vertices)[i] <= 0)
        {
            --i;
            --(*num_polys);
        }
    }

    // Copy over the vertex list; we assume RtInt == int
    FnAttribute::IntConstVector::const_iterator vertexIt = vertexListAttr.getNearestSample(0).begin();
    FnAttribute::IntConstVector::const_iterator vertexItEnd = vertexListAttr.getNearestSample(0).end();
    for(int i = 0; vertexIt != vertexItEnd; ++vertexIt, i++)
    {
        (*vertices)[i] = (RtInt)*vertexIt;
    }

    attrList->push_back(AttrListEntry(RI_P, RI_P, pointAttr, true));
    FnAttribute::FloatAttribute normalAttr = pointGroupAttr.getChildByName("N");
    if (normalAttr.isValid())
    {
        attrList->push_back(AttrListEntry(RI_N, RI_N, normalAttr, true));
    }
    else
    {
        if (vertexGroupAttr.isValid())
        {
            normalAttr = vertexGroupAttr.getChildByName("N");
            if (normalAttr.isValid())
            {
                attrList->push_back(AttrListEntry("facevarying normal N", RI_N, normalAttr, true));
            }
        }
    }

    FnAttribute::FloatAttribute uvAttr = pointGroupAttr.getChildByName("uv");
    if (uvAttr.isValid())
    {
        attrList->push_back(AttrListEntry("varying float[2] st", "st", uvAttr, false));
    }
    else
    {
        if (vertexGroupAttr.isValid())
        {
            uvAttr = vertexGroupAttr.getChildByName("uv");
            if (uvAttr.isValid())
            {
                attrList->push_back(AttrListEntry("facevarying float[2] st", "st", uvAttr, false));
            }
        }
    }

    return true;
}

namespace
{
    float falseFloat = 0;
    float trueFloat = 1.0f;
    int falseInt = 0;
    int trueInt = 1;
}


/////////////////////////////////////////////////////////////////////////////
//
// AttrList_Converter::AttrToParam
//
RtPointer AttrList_Converter::AttrToParam(Attribute attr, const std::string& decl, float time)
{
    //Check which attribute type attr is and convert it accordingly

    FnAttribute::IntAttribute intAttr = attr;
    if (intAttr.isValid())
    {
        return const_cast<int*>(intAttr.getNearestSample(time).data());
    }

    FnAttribute::FloatAttribute floatAttr = attr;
    if ( floatAttr.isValid())
    {
        return const_cast<float*>(floatAttr.getNearestSample(time).data());
    }

    FnAttribute::DoubleAttribute doubleAttr = attr;
    if (doubleAttr.isValid())
    {
        // Doubles need to be converted to floats.
        _floatVectors.push_back(std::vector<RtFloat>());
        std::vector<RtFloat>& floatVector = _floatVectors.back();

        FnAttribute::DoubleConstVector sample = doubleAttr.getNearestSample(time);
        floatVector.reserve(sample.size());

        for (DoubleConstVector::const_iterator it = sample.begin(); it != sample.end(); ++it)
            floatVector.push_back(static_cast<RtFloat>(*it));

        return const_cast<float*>(&floatVector[0]);
    }

    FnAttribute::StringAttribute stringAttr = attr;
    if (stringAttr.isValid())
    {
        // This processing on the declaration really makes me uneasy;
        // it does a substring match, rather than really working with the declaration.
        // If the name is something like 'string is_foo_an_int', then we'll get confused.
        // Regexp processing seems like the right thing to do here...  /shrug

        // If it's a string, but declared as a float, then use some 'special' conversion rules.
        if ( pystring::startswith( decl, "float " ) || decl.find(" float ") != std::string::npos)
        {
            if (std::string(stringAttr.getNearestSample(time)[0]) == "Yes")
            {
                return &trueFloat;
            }
            else
            {
                return &falseFloat;
            }
        }
        // If it's a string, but declared as an int, then use some 'special' conversion rules.
        else if ( pystring::startswith( decl, "int " ) || decl.find(" int ") != std::string::npos)
        {
            if (std::string(stringAttr.getNearestSample(time)[0]) == "Yes")
            {
                return &trueInt;
            }
            else
            {
                return &falseInt;
            }
        }
        else
        {
            //Just return the FnAttribute::StringAttribute's internal reference to the char** (using data() )
            FnAttribute::StringConstVector sample = stringAttr.getNearestSample(time);
            return (RtPointer) sample.data();
        }
    }

    // none of the above attr types were matched
    return 0x0;
}


/////////////////////////////////////////////////////////////////////////////
//
// AttrList_Converter::AttrList_Converter
//
AttrList_Converter::AttrList_Converter(const AttrList& attrList)
{
    init(attrList, std::set<float>());
}

AttrList_Converter::AttrList_Converter(const AttrList& attrList, const std::set<float>& samples)
{
    init(attrList, samples);
}

void AttrList_Converter::init(const AttrList& attrList, const std::set<float>& samples)
{
    std::set<float> sampleTimes = samples;

    for (AttrList::const_iterator it = attrList.begin(); it != attrList.end(); ++it)
    {
        //figure out which samples are relevant
        if (it->_mightBlur)
        {
            //check sample times of the attr
            DataAttribute attr = it->_value;
            if (!attr.isValid()) continue;

            int64_t numSampleTimes = attr.getNumberOfTimeSamples();
            std::set< float >::size_type sizePreSetAddition = numSampleTimes;
            const std::string attrName = it->_name;

            // If we are a static attribute - i.e. there is only one time sample
            // then we are going to ignore it...
            if (numSampleTimes > 1)
            {
                for (int64_t i = 0; i < numSampleTimes; ++i)
                {
                    sampleTimes.insert(attr.getSampleTime(i));
                }

                // Check for inconsistent sample times.
                // If the set was originally empty then addition is always
                // allowed, however, if the set is now larger, it means we have
                // inconsistent sample times against multiple attributes.
                if(!(sizePreSetAddition == 0) && (sampleTimes.size() - sizePreSetAddition) )
                {
                    std::stringstream ss;
                    ss << "Incoherent samples found on attribute "
                       << attrName;
                    Report_Warning(ss.str(), "");
                }
            }
        }
    }

    // We provide an anchor at t = 0...
    if (sampleTimes.size() == 0)
    {
        sampleTimes.insert( 0 );
    }

    for (std::set<float>::iterator sample_it = sampleTimes.begin(); sample_it != sampleTimes.end(); ++sample_it)
    {
        float sampleTime = *sample_it;

        _nameTokenMap[sampleTime] = std::vector<RtToken>();
        std::vector<RtToken>& nameTokens = _nameTokenMap[sampleTime];

        _declTokenMap[sampleTime] = std::vector<RtToken>();
        std::vector<RtToken>& declTokens = _declTokenMap[sampleTime];

        _paramMap[sampleTime] = std::vector<RtPointer>();
        std::vector<RtPointer>& params = _paramMap[sampleTime];

        for (AttrList::const_iterator it = attrList.begin(); it != attrList.end(); ++it)
        {
            RtPointer value = AttrToParam(it->_value, it->_declaration, sampleTime);

            nameTokens.push_back(const_cast<RtToken>(it->_name.c_str()));
            declTokens.push_back(const_cast<RtToken>(it->_declaration.c_str()));

            params.push_back(value);

        }

        if (nameTokens.empty())
        {
            _nameTokenMap.erase(sampleTime);
            _declTokenMap.erase(sampleTime);
            _paramMap.erase(sampleTime);
        }
        else
        {
            _sampleTimes.push_back(*sample_it);
        }
    }

    if (_sampleTimes.empty())
        _sampleTimes.push_back(0);
}

unsigned long AttrList_Converter::getNumSampleTimes() const
{
    return _sampleTimes.size();
}

unsigned long AttrList_Converter::getSize(float t)
{
    std::map<float, std::vector<RtToken> >::const_iterator it = _nameTokenMap.find(t);
    if (it == _nameTokenMap.end()) return 0;
    return it->second.size();
}

RtToken* AttrList_Converter::getNameTokens(float t)
{
    std::map<float, std::vector<RtToken> >::const_iterator it = _nameTokenMap.find(t);
    if (it == _nameTokenMap.end()) return 0;
    if (it->second.size() == 0) return 0;
    return const_cast<char**>(&(it->second[0]));
}

RtToken* AttrList_Converter::getDeclarationTokens(float t)
{
    std::map<float, std::vector<RtToken> >::const_iterator it = _declTokenMap.find(t);
    if (it == _declTokenMap.end()) return 0;
    if (it->second.size() == 0) return 0;
    return const_cast<char**>(&(it->second[0]));
}

RtPointer* AttrList_Converter::getParameters(float t)
{
    std::map<float, std::vector<RtPointer> >::const_iterator it = _paramMap.find(t);
    if (it == _paramMap.end()) return 0;
    if (it->second.size() == 0) return 0;
    return const_cast<RtPointer*>(&(it->second[0]));
}

void AddEntry(AttrList* attrList, AttrListEntry entry)
{
    for(AttrList::iterator it = attrList->begin(); it != attrList->end(); ++it)
    {
        if (it->_name == entry._name)
        {
            *it = entry;
            return;
        }
    }

    attrList->push_back(entry);
}

void AddTextureAttrs(AttrList* attrList, FnAttribute::GroupAttribute texturesAttr, int uniformCount, FnScenegraphIterator sgIterator)
{
    int numChildren = texturesAttr.getNumberOfChildren();

    for (int i = 0; i < numChildren; i++)
    {
        Attribute childAttr = texturesAttr.getChildByIndex(i);
        std::string childName = texturesAttr.getChildName(i);
        FnAttribute::StringAttribute stringAttr;

        // check to see if it's an index-based array instead of a standard array for maps
        // index-based arrays are GroupAttrs with indicies and maps children.
        FnAttribute::GroupAttribute groupAttr = childAttr;
        if (groupAttr.isValid())
        {
            FnAttribute::IntAttribute indicesAttr = groupAttr.getChildByName("indicies");
            if (!indicesAttr.isValid()) indicesAttr = groupAttr.getChildByName("indices");
            FnAttribute::StringAttribute mapsAttr = groupAttr.getChildByName("maps");
            if (indicesAttr.isValid() && mapsAttr.isValid())
            {
                FnAttribute::IntConstVector indexData = indicesAttr.getNearestSample(0);
                FnAttribute::StringConstVector mapData = mapsAttr.getNearestSample(0);

                std::vector<const char*> mapList;
                mapList.reserve(indexData.size());
                for (IntConstVector::const_iterator indexIt = indexData.begin(); indexIt != indexData.end(); ++indexIt)
                {
                    int index = *indexIt;
                    if ( index >= 0 && index < static_cast<int>(mapData.size()))
                    {
                        mapList.push_back(mapData[index]);
                    }
                    else
                    {
                        mapList.push_back(0x0);
                    }
                }
                stringAttr = FnAttribute::StringAttribute(&mapList[0], mapData.size(), 1);
            }
        }

        //if it is not an index-based array, then assume the childAttr is a FnAttribute::StringAttribute and use it
        if (!stringAttr.isValid())
        {
            stringAttr = childAttr;
            if (!stringAttr.isValid()) continue;
        }

        int size = stringAttr.getNearestSample(0).size();
        if (size > 1 && size != uniformCount)
        {
            Report_Warning("Mismatched texture array lengths.", sgIterator);
            stringAttr = FnAttribute::StringAttribute(stringAttr.getValue());
            size = 1;
        }

        std::string token = size > 1 ? "uniform string " : "constant string ";
        token += childName;


        AddEntry(attrList, AttrListEntry(token, childName, stringAttr, true));
    }
}

///////////////////////////////////////////////////////////////////////////////
///
void AddAttributesFromTextureAttrs(GroupAttribute texturesAttr)
{

    if (!texturesAttr.isValid())
    {
        return;
    }

    AttrList attrList;

    int numChildren = texturesAttr.getNumberOfChildren();

    for (int i = 0; i < numChildren; i++)
    {
        Attribute childAttr = texturesAttr.getChildByIndex(i);
        std::string childName = texturesAttr.getChildName(i);

        FnAttribute::StringAttribute stringAttr;
        FnAttribute::IntAttribute intAttr;

        // Check if the child attr is a GroupAttribute
        FnAttribute::GroupAttribute groupAttr = childAttr;
        if (groupAttr.isValid())
        {
            stringAttr = groupAttr.getChildByName("maps");
            intAttr = groupAttr.getChildByName("indicies");
            if (!intAttr.isValid()) intAttr = groupAttr.getChildByName("indices");
        }

        // Then the child attr might be a FnAttribute::StringAttribute...
        if (!stringAttr.isValid())
        {
            stringAttr = childAttr;
            if (!stringAttr.isValid()) continue;
        }

        std::string mapToken;
        if (stringAttr.getNearestSample(0).size() > 1 )
        {
            std::ostringstream buffer;

            buffer << "constant string[" <<
                    stringAttr.getNearestSample(0).size() <<
                    "] kmap__" << childName;

            mapToken = buffer.str();
        }
        else
        {
            mapToken = "constant string kmap__" + childName;
        }

        AddEntry(&attrList, AttrListEntry(mapToken, "kmap__" + childName,
                stringAttr, true));

        if (!intAttr.isValid())
        {
            intAttr = FnAttribute::IntAttribute(0);
        }

        std::string indexToken;
        if (intAttr.getNearestSample(0).size() > 1 )
        {
            std::ostringstream buffer;

            buffer << "constant int[" <<
                    intAttr.getNearestSample(0).size() <<
                    "] kindex__" << childName;

            indexToken = buffer.str();
        }
        else
        {
            indexToken = "constant int kindex__" + childName;
        }

        AddEntry(&attrList, AttrListEntry(indexToken,
                "kindex__"+childName, intAttr, true));

        //lengths and cache id
        AddEntry(&attrList, AttrListEntry("int kmaplen__" + childName,
                "kmaplen__" + childName, FnAttribute::IntAttribute(stringAttr.getNearestSample(0).size()), true));

        AddEntry(&attrList, AttrListEntry("int kindexlen__" + childName,
                "kindexlen__" + childName, FnAttribute::IntAttribute(intAttr.getNearestSample(0).size()), true));

        // todo: review if this is supposed to be like this
        std::ostringstream combinedHashes;
        combinedHashes << stringAttr.getHash().str();
        combinedHashes << ":" << std::endl;
        combinedHashes << intAttr.getHash().str();

        AddEntry(&attrList, AttrListEntry("string khash__" + childName,
                "khash__" + childName,
                FnAttribute::StringAttribute(combinedHashes.str()), true));
    }

    //no attr list?
    if (attrList.empty())
    {
        return;
    }

    AttrList_Converter converter(attrList);
    RiAttributeV(const_cast<char*>("user"),
                    converter.getSize(0),
                    converter.getDeclarationTokens(0),
                    converter.getParameters(0));
}

namespace
{
    // Build prman type token (class specifier + value type + element size)
    std::string GetPRManTypeFromArbitraryOutputAttr(
        const FnGeolibServices::ArbitraryOutputAttr & arbitraryOutputAttr,
        const std::string & geometryType)
    {
        std::string prmanType;

        //  class specifier
        std::string scope = arbitraryOutputAttr.getScope();
        if (scope == "primitive")
        {
            prmanType = "constant ";
        }
        else if (geometryType == "polymesh" || geometryType == "subdmesh")
        {
            if (scope == "face") prmanType = "uniform ";
            else if (scope == "vertex") prmanType = "facevarying ";
            else if (scope == "point")
            {
                std::string interpolationType = arbitraryOutputAttr.getInterpolationType();
                if( interpolationType == "subdiv" )
                    prmanType = "vertex ";
                else
                    prmanType = "varying ";
            }
        }
        else if (geometryType == "nurbspatch")
        {
            if (scope == "face") prmanType = "varying ";
            else if (scope == "vertex") prmanType = "vertex ";
            else if (scope == "point") prmanType = "vertex ";
        }
        else if (geometryType == "pointcloud")
        {
            if (scope == "face") prmanType = "varying ";
            else if (scope == "vertex") prmanType = "varying ";
            else if (scope == "point") prmanType = "varying ";
        }
        else if (geometryType == "curves" || geometryType == "volume")
        {
            if (scope == "point") prmanType = "varying ";
            else if (scope == "vertex") prmanType = "vertex ";
            else if (scope == "face") prmanType = "uniform ";
        }



        int elementSize = arbitraryOutputAttr.getElementSize();

        //  value type
        std::string outputType = arbitraryOutputAttr.getOutputType();
        if (outputType == "float") prmanType += "float";
        else if (outputType == "double") prmanType += "float"; // (yes, double->float)
        else if (outputType == "string") prmanType += "string";
        else if (outputType == "int") prmanType += "int";
        else if (outputType == "color3") prmanType += "color";
        else if (outputType == "normal3") prmanType += "normal";
        else if (outputType == "vector3") prmanType += "vector";
        else if (outputType == "point2")
        {
            prmanType += "float";
            elementSize *= 2;
        }
        else if (outputType == "point3") prmanType += "point";
        else if (outputType == "point4") prmanType += "hpoint";
        else if (outputType == "matrix16") prmanType += "matrix";
        else return "";

        if (arbitraryOutputAttr.isArray() || elementSize != 1)
        {
            std::ostringstream extra;
            extra << "[" << elementSize << "]";
            prmanType += extra.str();
        }

        return prmanType;
    }
}

void AddArbitraryParameters(AttrList* attrList, FnAttribute::GroupAttribute geometryAttr, FnScenegraphIterator sgIterator)
{
    ///////////////////////////////////////////////////////////////////////////////////////
    // geometry.arbitrary
    ///////////////////////////////////////////////////////////////////////////////////////
    FnAttribute::GroupAttribute arbitraryAttr = geometryAttr.getChildByName("arbitrary");
    if (arbitraryAttr.isValid())
    {
        int numChildren = arbitraryAttr.getNumberOfChildren();

        for (int i = 0; i < numChildren; i++)
        {
            FnAttribute::GroupAttribute groupEntryAttr = arbitraryAttr.getChildByIndex(i);
            std::string childName = arbitraryAttr.getChildName(i);

            if (!groupEntryAttr.isValid())
            {
                Report_Warning("Unrecognized arbitrary parameter '" + childName + "'.", sgIterator);
                continue;
            }

            std::string geometryType = sgIterator.getType();
            if (geometryType == "spheres")
            {
                geometryType = "pointcloud";
            }

            FnGeolibServices::ArbitraryOutputAttr arbitraryOutputAttr(
                childName, groupEntryAttr, geometryType, geometryAttr);

            if (!arbitraryOutputAttr.isValid())
            {
                Report_Warning("Unrecognized arbitrary parameter '" + childName + "': "
                               + arbitraryOutputAttr.getError() + ".", sgIterator);
                continue;
            }

            if (arbitraryOutputAttr.getElementSize() == 0 &&
                !arbitraryOutputAttr.isArray())
            {
                Report_Warning("Empty arbitrary parameter '" + childName + "'.",
                               sgIterator);
                continue;
            }

            // Build token
            std::string token = GetPRManTypeFromArbitraryOutputAttr(arbitraryOutputAttr, geometryType);
            if (token.empty())
            {
                continue; // silently skip this attribute if prman type can't be determined
            }
            token += " " + childName;

            // Get value attribute
            std::string outputType = arbitraryOutputAttr.getOutputType();
            Attribute valueAttr;
            if (outputType == "string" || outputType == "int")
            {
                valueAttr = arbitraryOutputAttr.getValueAttr();
            }
            else
            {
                valueAttr = arbitraryOutputAttr.getValueAttr("float");
            }
            if (!valueAttr.isValid())
            {
                Report_Warning("Unrecognized arbitrary parameter '" + childName + "': "
                               + arbitraryOutputAttr.getError() + ".", sgIterator);
                continue;
            }

            AddEntry(attrList,  AttrListEntry(token, childName, valueAttr, true));
        }
    }
}


void AddRefParameters(AttrList* attrList, FnAttribute::GroupAttribute geometryAttr, FnScenegraphIterator sgIterator)
{
    FnAttribute::GroupAttribute pointAttr = geometryAttr.getChildByName("point");
    if (!pointAttr.isValid()) return;

    FnAttribute::FloatAttribute prefAttr = pointAttr.getChildByName("Pref");
    if (prefAttr.isValid())
    {
        int tupleSize = prefAttr.getTupleSize();
        if (tupleSize == 3 || tupleSize == 4)
        {
            std::string token;
            if (tupleSize == 3)
            {
                token = "vertex point Pref";
            }
            else if (tupleSize == 4)
            {
                token = "vertex hpoint Pref";
            }
            AddEntry(attrList, AttrListEntry(token, "Pref", prefAttr, true));
        }
        else
        {
            Report_Warning("Pref attribute has an invalid tuple size.", sgIterator);
        }
    }

    FnAttribute::FloatAttribute nrefAttr = pointAttr.getChildByName("Nref");
    if (nrefAttr.isValid())
    {
        if (nrefAttr.getTupleSize() == 3)
        {
            std::string token = "varying normal Nref";
            AddEntry(attrList, AttrListEntry(token, "Nref", nrefAttr, true));
        }
        else
        {
            Report_Warning("Nref attribute has an invalid tuple size.", sgIterator);
        }
    }
}


void FillBoundFromAttr( RtBound * bound, FnAttribute::DoubleAttribute boundAttr, PRManPluginState* sharedState )
{
    int64_t numSampleTimes = boundAttr.getNumberOfTimeSamples();

    bool isFirst = true;
    for ( int64_t index = 0; index < numSampleTimes; ++index)
    {
        float sampleTime = boundAttr.getSampleTime(index);
        for ( int i = 0; i < 6; ++i )
        {
            float value = boundAttr.getNearestSample(sampleTime)[i];

            if ( isFirst )
            {
                Report_Debug("calculating bounds: first frame ", sharedState, FnScenegraphIterator());
               (*bound)[i] = value;
            }
            else
            {
                Report_Debug("calculating bounds: inflating bounds for a time sample", sharedState, FnScenegraphIterator());

                //odd number is max case
                if ( i%2 )
                {
                    (*bound)[i] = std::max((*bound)[i], value);
                }
                //even number is the min case
                else
                {
                    (*bound)[i] = std::min((*bound)[i], value);
                }
            }
        }

        isFirst = false;
    }
}

bool IsVisible(FnScenegraphIterator sgIterator)
{
    FnAttribute::IntAttribute visibleAttr = sgIterator.getAttribute("visible", true);

    if (!visibleAttr.isValid())
        return true;

    return visibleAttr.getValue() != 0;
}

CameraInfoStyle ParseCameraInfoAttributes(FnScenegraphIterator sgIterator,
                                          std::string & outputCameraPath,
                                          const std::string & attrBaseName)
{
    CameraInfoStyle cameraInfoStyle = CAMERA_NONE;
    outputCameraPath = "";

    //check camera inclusion info
    FnAttribute::StringAttribute includeCameraInfoAttr = sgIterator.getAttribute(attrBaseName+".includeCameraInfo");
    if ( includeCameraInfoAttr.isValid() && includeCameraInfoAttr.getValue() != "None")
    {
        if ( includeCameraInfoAttr.getValue() == "As Parameters" )
        {
            cameraInfoStyle = CAMERA_PARAMETERS;
        }
        else if ( includeCameraInfoAttr.getValue() == "As Attributes" )
        {
            cameraInfoStyle = CAMERA_ATTRIBUTES;
        }

        bool useRenderCamera = true;

        FnAttribute::GroupAttribute cameraInfoAttr = sgIterator.getAttribute(attrBaseName+".cameraInfo");

        if ( cameraInfoAttr.isValid() )
        {
            FnAttribute::StringAttribute whichCameraAttr = cameraInfoAttr.getChildByName("whichCamera");

            if ( whichCameraAttr.isValid() && whichCameraAttr.getValue() == "Other Camera" )
            {
                FnAttribute::StringAttribute cameraPathAttr = cameraInfoAttr.getChildByName("cameraPath");

                if ( cameraPathAttr.isValid() )
                {
                    useRenderCamera = false;
                    outputCameraPath = cameraPathAttr.getValue();
                }
            }
        }

        if ( useRenderCamera )
        {
            FnScenegraphIterator rootSgItearator = sgIterator.getRoot();

            if ( rootSgItearator.isValid() )
            {
                FnAttribute::StringAttribute cameraNameAttr = rootSgItearator.getAttribute("renderSettings.cameraName");

                if ( cameraNameAttr.isValid() )
                {
                    outputCameraPath = cameraNameAttr.getValue();
                }
                else
                {
                    outputCameraPath = "/root/world/cam/camera";
                }
            }
        }
    }

    return cameraInfoStyle;
}

float GetFrameNumber( FnScenegraphIterator sgIterator, PRManPluginState* sharedState )
{
    float frameNumber = sharedState->proceduralSettings.frameNumber;

    FnAttribute::FloatAttribute frameTimeAttr  = sgIterator.getAttribute("time.frameTime", true);

    if ( frameTimeAttr.isValid() )
    {
        frameNumber = frameTimeAttr.getValue();
    }

    return frameNumber;
}

// Builds a declaration string for PRMan based on an attribute.
// For example, passing a float attribute with 4 values and an argName 'myFloat'
// will return the following: float[4] myFloat
std::string BuildDeclarationFromAttribute(const std::string& argName, const FnKat::Attribute attr)
{
    const FnKatAttributeType type = attr.getType();
    const char * typeStrs[] = { "", "int", "float", "double", "string"};
    const unsigned int numValues = ((FnKat::DataAttribute)attr).getNumberOfValues();
    std::stringstream declaration;

    if (numValues == 0)
    {
        return "";
    }

    declaration << typeStrs[type];
    if (numValues > 1)
    {
        declaration << "[" << numValues << "]";
    }
    declaration << " " << argName;
    return declaration.str();
}


// Basic support for creating an object instance, given a handle.
// If a scenegraphIterator is also present, it will be inspected for
// attributes under instance.arbitrary to override instance primvars.
// Note: The function assumes that the producerPacket has an objectHandle set.
void HandleObjectInstance( const ProducerPacket& producerPacket )
{
    RtToken *tokens = NULL;
    RtPointer *values = NULL;
    unsigned int numParams = 0;

    if (producerPacket.producerType != kObjectInstance || producerPacket.producerData == NULL)
    {
        return;
    }

    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    if (sgIterator.isValid())
    {
        // We allow primvar overrides using instance.arbitrary syntax as per geometry.arbitrary
        FnAttribute::GroupAttribute instanceGroup = sgIterator.getAttribute("instance", true);
        if (instanceGroup.isValid())
        {
            AttrList attrList;
            AddArbitraryParameters(&attrList, instanceGroup, sgIterator);
            AttrList_Converter converter(attrList);
            std::vector<float> &sampleTimes = converter.getSampleTimes();

            if( sampleTimes.size() > 0 )
            {
                const float firstSample = sampleTimes[0];
                numParams = converter.getSize(firstSample);
                tokens = converter.getDeclarationTokens(firstSample);
                values = converter.getParameters(firstSample);
                RiObjectInstanceV( (RtObjectHandle)producerPacket.producerData, numParams, tokens, values );
                return;
            }
        }
    }

    RiObjectInstanceV( (RtObjectHandle)producerPacket.producerData, numParams, tokens, values );
}

} // namespace PRManProcedural
