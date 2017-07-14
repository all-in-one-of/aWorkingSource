// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <ArnoldRendererInfoDump.h>

#include <FnRendererInfo/suite/RendererObjectDefinitions.h> // need the kFn* constants

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>


#include <ai.h>
#include <ai_deprecated.h>

#ifdef _WIN32
#include <windowshelpers.h>
#else
#include <sys/wait.h>
#include <libgen.h>
#endif //_WIN32

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <sstream>

#include <string>
#include <vector>
#include <set>

#include <iostream>
#include <fstream>

#include <pystring/pystring.h>

// make arnold quiet, don't unload plugins on exit
void setArnoldOptions()
{
    //set log options for this run of Arnold
    //(turn on warnings/errors so we can see if there are dlopen errors on shader load)
    int flags = AI_LOG_WARNINGS | AI_LOG_ERRORS;
    AiMsgSetConsoleFlags(flags);
    AiMsgSetMaxWarnings(100);
}

bool isDir(const std::string &path)
{
    struct stat filestat;
    return (stat(path.c_str(), &filestat)==0 && S_ISDIR(filestat.st_mode));
}

int convertArnoldType(int arnoldType)
{
    switch (arnoldType)
    {
        case AI_TYPE_BYTE:      return kFnRendererObjectValueTypeByte;
        case AI_TYPE_INT:       return kFnRendererObjectValueTypeInt;
        case AI_TYPE_UINT:      return kFnRendererObjectValueTypeUint;
        case AI_TYPE_BOOLEAN:   return kFnRendererObjectValueTypeBoolean;
        case AI_TYPE_FLOAT:     return kFnRendererObjectValueTypeFloat;
        case AI_TYPE_RGB:       return kFnRendererObjectValueTypeColor3;
        case AI_TYPE_RGBA:      return kFnRendererObjectValueTypeColor4;
        case AI_TYPE_VECTOR:    return kFnRendererObjectValueTypeVector3;
        case AI_TYPE_POINT:     return kFnRendererObjectValueTypePoint3;
        case AI_TYPE_POINT2:    return kFnRendererObjectValueTypePoint2;
        case AI_TYPE_STRING:    return kFnRendererObjectValueTypeString;
        case AI_TYPE_MATRIX:    return kFnRendererObjectValueTypeMatrix;
        case AI_TYPE_ENUM:      return kFnRendererObjectValueTypeEnum;
        case AI_TYPE_POINTER:   return kFnRendererObjectValueTypePointer;
        case AI_TYPE_NODE:      return kFnRendererObjectValueTypeLocation;

        case AI_TYPE_ARRAY:
        case AI_TYPE_UNDEFINED:
        default:
            return kFnRendererObjectValueTypeUnknown;
    }
}

typedef std::pair<std::string, int> EnumPair;
typedef std::vector<EnumPair> EnumPairVector;

void buildParamsForNode(FnAttribute::GroupBuilder &gb,
                        const std::string & prefix,
                        const AtNodeEntry* nodeEntry)
{
    int num_params = AiNodeEntryGetNumParams(nodeEntry);
    for (int p = 0; p < num_params; ++p)
    {
        const AtParamEntry* pentry = AiNodeEntryGetParameter(nodeEntry, p);
        if (!pentry) continue;

        int paramType = AiParamGetType(pentry);
        const AtParamValue* paramValue = AiParamGetDefault(pentry);
        std::string paramName = AiParamGetName(pentry);
        EnumPairVector paramEnums;
        if (paramName == "name") continue;

        int type;
        AtArray * array = NULL;
        int arraySize = 1;

        if (paramType == AI_TYPE_ARRAY)
        {
            array = paramValue->ARRAY;
            type = convertArnoldType(array->type);
            arraySize = array->nelements;
        }
        else
        {
            type = convertArnoldType(paramType);
        }

        FnAttribute::Attribute defaultAttr;
        FnAttribute::Attribute hintsAttr;

        //get default
        if (paramValue)
        {
            switch (type)
            {
                case kFnRendererObjectValueTypeInt:
                {
                    FnAttribute::IntBuilder intBuilder(1);
                    if (!array)
                    {
                        intBuilder.push_back(paramValue->INT);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            intBuilder.push_back(AiArrayGetInt(array, i));
                        }
                    }
                    defaultAttr = intBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeUint:
                {
                    FnAttribute::IntBuilder intBuilder(1);
                    if (!array)
                    {
                        intBuilder.push_back(static_cast<int>(
                                paramValue->UINT));
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            intBuilder.push_back(static_cast<int>(
                                    AiArrayGetUInt(array, i)));
                        }
                    }
                    defaultAttr = intBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeBoolean:
                {
                    FnAttribute::IntBuilder intBuilder(1);
                    if (!array)
                    {
                        intBuilder.push_back(static_cast<int>(
                                paramValue->BOOL));
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            intBuilder.push_back(static_cast<int>(
                                    AiArrayGetBool(array, i)));
                        }
                    }
                    defaultAttr = intBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeByte:
                {
                    FnAttribute::IntBuilder intBuilder(1);
                    if (!array)
                    {
                        intBuilder.push_back(static_cast<int>(
                                paramValue->BYTE));
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            intBuilder.push_back(static_cast<int>(
                                    AiArrayGetByte(array, i)));
                        }
                    }
                    defaultAttr = intBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeFloat:
                {
                    FnAttribute::FloatBuilder floatBuilder(1);
                    if (!array)
                    {
                        floatBuilder.push_back(paramValue->FLT);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            floatBuilder.push_back(AiArrayGetFlt(array, i));
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeVector3:
                {
                    FnAttribute::FloatBuilder floatBuilder(3);
                    if (!array)
                    {
                        floatBuilder.push_back(paramValue->VEC.x);
                        floatBuilder.push_back(paramValue->VEC.y);
                        floatBuilder.push_back(paramValue->VEC.z);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            AtVector v = AiArrayGetVec(array, i);
                            floatBuilder.push_back(v.x);
                            floatBuilder.push_back(v.y);
                            floatBuilder.push_back(v.z);
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypePoint2:
                {
                    FnAttribute::FloatBuilder floatBuilder(2);
                    if (!array)
                    {
                        floatBuilder.push_back(paramValue->PNT2.x);
                        floatBuilder.push_back(paramValue->PNT2.y);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            AtPoint2 p2 = AiArrayGetPnt2(array, i);
                            floatBuilder.push_back(p2.x);
                            floatBuilder.push_back(p2.y);
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypePoint3:
                {
                    FnAttribute::FloatBuilder floatBuilder(3);
                    if (!array)
                    {
                        floatBuilder.push_back(paramValue->PNT.x);
                        floatBuilder.push_back(paramValue->PNT.y);
                        floatBuilder.push_back(paramValue->PNT.z);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            AtPoint p = AiArrayGetPnt(array, i);
                            floatBuilder.push_back(p.x);
                            floatBuilder.push_back(p.y);
                            floatBuilder.push_back(p.z);
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeColor3:
                {
                    FnAttribute::FloatBuilder floatBuilder(3);
                    if (!array)
                    {
                        floatBuilder.push_back(paramValue->RGB.r);
                        floatBuilder.push_back(paramValue->RGB.g);
                        floatBuilder.push_back(paramValue->RGB.b);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            AtRGB rgb = AiArrayGetRGB(array, i);
                            floatBuilder.push_back(rgb.r);
                            floatBuilder.push_back(rgb.g);
                            floatBuilder.push_back(rgb.b);
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeColor4:
                {
                    FnAttribute::FloatBuilder floatBuilder(4);
                    if (!array)
                    {
                        floatBuilder.push_back(paramValue->RGBA.r);
                        floatBuilder.push_back(paramValue->RGBA.g);
                        floatBuilder.push_back(paramValue->RGBA.b);
                        floatBuilder.push_back(paramValue->RGBA.a);
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            AtRGBA rgb = AiArrayGetRGBA(array, i);
                            floatBuilder.push_back(rgb.r);
                            floatBuilder.push_back(rgb.g);
                            floatBuilder.push_back(rgb.b);
                            floatBuilder.push_back(rgb.a);
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeString:
                {
                    FnAttribute::StringBuilder stringBuilder(1);
                    if (!array)
                    {
                        stringBuilder.push_back(std::string(
                                paramValue->STR));
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            const char * value = AiArrayGetStr(array, i);
                            if(!value)
                            {
                                value = "";
                            }
                            stringBuilder.push_back(std::string(value));
                        }
                    }
                    defaultAttr = stringBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeMatrix:
                {
                    FnAttribute::FloatBuilder floatBuilder(4);
                    if (!array)
                    {
                        AtMatrix* m = paramValue->pMTX;
                        for (int y = 0; y < 4; ++y)
                        {
                            for (int x = 0; x < 4; ++x)
                            {
                                floatBuilder.push_back((*m)[x][y]);
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < arraySize; ++i)
                        {
                            AtMatrix m;
                            AiArrayGetMtx(array, i, m);

                            for (int y = 0; y < 4; ++y)
                            {
                                for (int x = 0; x < 4; ++x)
                                {
                                    floatBuilder.push_back(m[x][y]);
                                }
                            }
                        }
                    }
                    defaultAttr = floatBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeEnum:
                {
                    FnAttribute::IntBuilder intBuilder(1);
                    // Only for non-array
                    if (!array)
                    {
                        intBuilder.push_back(paramValue->INT);
                    }
                    defaultAttr = intBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypePointer:
                case kFnRendererObjectValueTypeLocation:
                {
                    FnAttribute::StringBuilder stringBuilder(1);
                    for (int i = 0; i < arraySize; ++i)
                    {
                        stringBuilder.push_back(std::string(""));
                    }
                    defaultAttr = stringBuilder.build();
                    break;
                }

                case kFnRendererObjectValueTypeUnknown:
                default:
                    break;
            }
        }

        // Get enum strings (non array params only)
        if (!array && type == kFnRendererObjectValueTypeEnum)
        {
            AtEnum enumType = AiParamGetEnum(pentry);
            for (int i = 0; ; i++)
            {
                const char* thisName = AiEnumGetString(enumType, i);
                if (!thisName) break;
                paramEnums.push_back(EnumPair(thisName, i));
            }
        }

        // If defaultAttr was not set so far, then we want to show that in the
        // parameter widget using a StringAttribute. An exception will be
        // opened for the case in which the parameter is an array with no
        // elements.
        if ( !defaultAttr.isValid() && (!array || (array && arraySize > 0)) )
        {
            defaultAttr =
                    FnAttribute::StringAttribute("UNIMPLEMENTED PARAM TYPE");
        }

        // Build widget hints based on array size and parameter type
        if (array)
        {
            FnAttribute::GroupBuilder hints;

            if (type == kFnRendererObjectValueTypeFloat
                || type == kFnRendererObjectValueTypeString
                || type == kFnRendererObjectValueTypeInt
                || type == kFnRendererObjectValueTypeUint
                || type == kFnRendererObjectValueTypeByte
                || type == kFnRendererObjectValueTypeBoolean
                || type == kFnRendererObjectValueTypePointer
                || type == kFnRendererObjectValueTypeLocation
                )
            {
                hints.set("widget", FnAttribute::StringAttribute("sortableArray"));
            }
            else
            {
                hints.set("widget", FnAttribute::StringAttribute("dynamicArray"));
            }


            if (type == kFnRendererObjectValueTypeColor3
                || type == kFnRendererObjectValueTypeColor4)
            {
                hints.set("panelWidget", FnAttribute::StringAttribute("color"));
            }
            else if (type == kFnRendererObjectValueTypeBoolean)
            {
                hints.set("childHints.widget",
                        FnAttribute::StringAttribute("checkBox"));
            }
            else if (type == kFnRendererObjectValueTypeMatrix)
            {
                hints.set("tupleGroupSize", FnAttribute::IntAttribute(4));
            }
            else if (type == kFnRendererObjectValueTypeInt
                     || type == kFnRendererObjectValueTypeUint
                     || type == kFnRendererObjectValueTypeByte)
            {
                hints.set("childHints.int", FnAttribute::IntAttribute(1));
            }
            else if (type == kFnRendererObjectValueTypePointer
                     || type == kFnRendererObjectValueTypeLocation)
            {
                hints.set("childHints.widget",
                        FnAttribute::StringAttribute("scenegraphLocation"));
            }

            hints.set("isDynamicArray", FnAttribute::IntAttribute(1));

            hintsAttr = hints.build();
        }

        std::string paramPrefix = prefix + kFnRendererObjectInfoKeyParam;
        paramPrefix += ".";
        paramPrefix += paramName;
        paramPrefix += ".";

        gb.set(paramPrefix + kFnRendererObjectInfoKeyParamType,
               FnAttribute::IntAttribute(type));

        gb.set(paramPrefix + kFnRendererObjectInfoKeyParamArraySize,
               FnAttribute::IntAttribute(arraySize));

        gb.set(paramPrefix + kFnRendererObjectInfoKeyParamDefault,
               defaultAttr);

        gb.set(paramPrefix + kFnRendererObjectInfoKeyParamHints,
               hintsAttr);

        if (!paramEnums.empty())
        {
            FnAttribute::GroupBuilder gb2;
            for (EnumPairVector::const_iterator iter=paramEnums.begin();
                    iter!=paramEnums.end(); ++iter)
            {
                gb2.set((*iter).first, FnAttribute::IntAttribute((*iter).second));
            }
            gb.set(paramPrefix + kFnRendererObjectInfoKeyParamEnums, gb2.build());
        }
    }
}

void buildNode(FnAttribute::GroupBuilder &gb,
               const AtNodeEntry *nodeEntry,
               const std::string &pluginPath)
{
    std::string name = AiNodeEntryGetName(nodeEntry);
    std::string prefix = name + ".";

    int typeInt = AiNodeEntryGetType(nodeEntry);
    std::string typeStr;
    std::vector<std::string> typeTags;
    switch (typeInt)
    {
    case AI_NODE_SHADER:
        typeStr = kFnRendererObjectTypeShader;
        break;
    case AI_NODE_LIGHT:
        typeStr = kFnRendererObjectTypeShader;
        typeTags.push_back("light");
        break;
    case AI_NODE_FILTER:
        typeStr = kFnRendererObjectTypeFilter;
        break;
    case AI_NODE_DRIVER:
        typeStr = kFnRendererObjectTypeDriver;
        break;
    default:
        break;
    }

    std::string arnoldtree;
    const char *arnoldtreeEnv = ::getenv("ARNOLDTREE");
    if (arnoldtreeEnv)
    {
        arnoldtree = arnoldtreeEnv;
    }

    std::string location;
    std::string fakeFileName;
    const char *filename = AiNodeEntryGetFilename(nodeEntry);
    if (filename == NULL)
    {
        // Fictitious file name if this is a built-in shader based on the
        // plug-in path. This will allow the built-in shader's args files to be
        // found in [pluginPath]/Args. pluginPath will contain the Libs
        // directory on it, which is removed from the path.
        size_t pathSize = pluginPath.size();
        if ( pluginPath.substr(pathSize - 5, pathSize) == "/Libs" )
        {
            fakeFileName = pluginPath.substr(0, pathSize - 5);
        }
        else
        {
            fakeFileName = pluginPath;
        }
        fakeFileName += "/" + name + ".built-in";
        filename = fakeFileName.c_str();

        location = arnoldtree;
    }
    else
    {
        char *tmplocation = ::strdup(filename);
        location = ::dirname(tmplocation);
        ::free(tmplocation);
    }

    int outputType = convertArnoldType(AiNodeEntryGetOutputType(nodeEntry));

    gb.set(prefix+kFnRendererObjectInfoKeyType, FnAttribute::StringAttribute(typeStr));
    if (typeTags.size())
    {
        gb.set(prefix+kFnRendererObjectInfoKeyTypeTags,
            FnAttribute::StringAttribute(typeTags));
    }
    gb.set(prefix+kFnRendererObjectInfoKeyLocation, FnAttribute::StringAttribute(location));
    gb.set(prefix+kFnRendererObjectInfoKeyFullPath, FnAttribute::StringAttribute(filename));
    gb.set(prefix+kFnRendererObjectInfoKeyOutputType, FnAttribute::IntAttribute(outputType));

    buildParamsForNode(gb, prefix, nodeEntry);
}

bool writeShaderInfo(std::ostream & os, std::string& pluginPath)
{
    AiBegin();
    setArnoldOptions();

    // If there are shaders in the
    // ARNOLD_SHADERLIB_PATH
    // then load them
    char* shaderLibEnv = ::getenv("ARNOLD_SHADERLIB_PATH");
    if (shaderLibEnv)
    {
        //tokenize
        std::vector<std::string> shaderLibPaths;
        pystring::split( shaderLibEnv, shaderLibPaths, ":" );
        if (!shaderLibPaths.size()) return false;

        //make sure local dir is in searchpaths
        shaderLibPaths.push_back(".");

        //Load plugin libraries, possibly from multiple paths
        for (std::vector<std::string>::iterator iter=shaderLibPaths.begin();
             iter!=shaderLibPaths.end(); ++iter)
        {
            if (!isDir(*iter)) continue;
            AiLoadPlugins((char*)(*iter).c_str());
        }
    }

    FnAttribute::GroupBuilder gb;
    gb.setGroupInherit(false);

    // build cache
    AtNodeEntryIterator *nodeEntryIt = AiUniverseGetNodeEntryIterator(AI_NODE_ALL);
    const AtNodeEntry* nodeEntry = AiNodeEntryIteratorGetNext(nodeEntryIt);
    while (nodeEntry != NULL)
    {
        buildNode(gb, nodeEntry, pluginPath);
        nodeEntry = AiNodeEntryIteratorGetNext(nodeEntryIt);
    }
    AiNodeEntryIteratorDestroy(nodeEntryIt);

    os << gb.build().getXML() << std::endl;

    AiEnd();

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "ArnoldRendererInfoDump: wrong number of args." << std::endl;
        std::cerr << "  Usage: " << argv[0] << " [outFile] [pluginPath])" << std::endl;
        return 1;
    }

    std::ofstream outfile(argv[1]);
    if (outfile.fail())
    {
        std::cerr << "ArnoldRendererInfoDump: error opening output file: " << argv[1] << std::endl;
        return 1;
    }

    std::string pluginPath(argv[2]);
    if (pluginPath.empty())
    {
        std::cerr << "ArnoldRendererInfoDump: plugin path passed as 2nd argument" << std::endl;
        return 1;
    }

    char* katanaRoot = getenv("KATANA_ROOT");
    if (katanaRoot == 0x0)
    {
        std::cerr << "ArnoldRendererInfoDump: KATANA_ROOT env var not set!" << std::endl;
        return 1;
    }

    std::string katanaRootStr(katanaRoot);
    if (!FnAttribute::Bootstrap(katanaRootStr))
    {
        std::cerr << "ArnoldRendererInfoDump: error bootstrapping attribute host" << std::endl;
        return 1;
    }

    bool success = writeShaderInfo(outfile, pluginPath);
    if (outfile.fail() || !success)
    {
        std::cerr << "ArnoldRendererInfoDump: error writing to output file" << std::endl;
        return 1;
    }
    outfile.close();

    return 0;
}
