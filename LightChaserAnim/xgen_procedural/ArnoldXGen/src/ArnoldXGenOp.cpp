// Description: An Op that creates a renderer procedural location for the XGen Arnold procedural plugin DSO
// Author: mike
// Date: 2016-12-1

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#else
#include <dirent.h>
#endif

#include <OpenEXR/ImathMatrix.h>

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolibServices/FnXFormUtil.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnAttributeUtils.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>


namespace
{


#ifdef _WIN32


inline std::string getenvHelper(const char* key)
{
    char var[4096];
    size_t used = 0;
    std::string result;
    if (getenv_s(&used, var, key) == 0 && used > 0)
    {
        var[used] = 0;
        result = var;
    }
    return result;
}

inline bool fileExists(const std::string& name)
{
    struct _stat32 buffer;
    return (_stat32(name.c_str(), &buffer) == 0);
}


inline void platformizePath(std::string& path, bool forceUnix = false)
{
    for (size_t i = 0; i < path.length(); ++i)
    {
        if (forceUnix)
        {
            if (path[i] == '\\')
                path[i] = '/';
        }
        else if (path[i] == '/')
            path[i] = '\\';
    }
}

std::vector<std::string> findXGenFiles(const std::string& pathWithFinalSlash, const std::string& scene)
{
    std::string platPath = pathWithFinalSlash;
    platformizePath(platPath);
    std::vector<std::string> xgenFiles;

    platPath += "*";
    WIN32_FIND_DATAA ffd;
    HANDLE hFindFile = FindFirstFileA(platPath.c_str(), &ffd);
    if (hFindFile == INVALID_HANDLE_VALUE || !hFindFile)
        return xgenFiles;

    std::string sceneLower = scene;
    for (size_t i = 0; i < sceneLower.length(); ++i)
        sceneLower[i] = (char)std::tolower(sceneLower[i]);
    if (scene.length() > 0)
        sceneLower += "__";

    do
    {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::string filename = ffd.cFileName;
            for (size_t i = 0; i < filename.length(); ++i)
                filename[i] = (char)std::tolower(filename[i]);
            if (filename.length() <= scene.length() + 2 + 5) // <scene>__*.xgen is what we're looking for
                continue;
            std::string extension = filename.substr(filename.length() - 5, 5);
            std::string prefix = filename.substr(0, sceneLower.length());
            if (extension == ".xgen" && prefix == sceneLower)
                xgenFiles.push_back(pathWithFinalSlash + ffd.cFileName);
        }
    } while (FindNextFileA(hFindFile, &ffd));
    FindClose(hFindFile);
    return xgenFiles;
}


#else

// Linux, OS X, etc

inline std::string getenvHelper(const char* key)
{
    const char* result = ::getenv(key);
    return result ? std::string(result) : std::string();
}

inline bool fileExists(const std::string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

// silence "unused parameter 'forceUnix'" warning
void platformizePath(std::string& path, bool forceUnix = false);

inline void platformizePath(std::string& path, bool)
{
    for (size_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }
}

std::vector<std::string> findXGenFiles(const std::string& pathWithFinalSlash, const std::string& scene)
{
    std::string platPath = pathWithFinalSlash;
    platformizePath(platPath);
    std::vector<std::string> xgenFiles;

    struct dirent *pDirent;
    DIR *pDir;
    pDir = opendir(platPath.c_str());
    if (pDir == NULL)
        return xgenFiles;
    while ((pDirent = readdir(pDir)) != NULL)
    {
#ifdef _DIRENT_HAVE_D_TYPE
        if (pDirent->d_type != DT_REG)
            continue;
#endif
        std::string filename = pDirent->d_name;
        if (filename.length() <= scene.length() + 2 + 5) // <scene>__*.xgen is what we're looking for
            continue;
        std::string extension = filename.substr(filename.length() - 5, 5);
        std::string prefix = scene.empty() ? std::string() : filename.substr(0, scene.length() + 2);
        if (extension == ".xgen" && prefix == (scene.empty() ? std::string() : scene + "__"))
            xgenFiles.push_back(pathWithFinalSlash + filename);
    }
    closedir(pDir);
    return xgenFiles;
}


#endif


inline std::string stripFromEnd(const std::string& s, const std::string& toStrip)
{
    if (s.length() < toStrip.length())
        return s;
    std::string ending = s.substr(s.length() - toStrip.length(), toStrip.length());
    if (ending == toStrip)
        return s.substr(0, s.length() - toStrip.length());
    return s;
}

inline bool endsWith(const std::string& s, const std::string& terminator)
{
    if (s.length() < terminator.length())
        return false;
    std::string ending = s.substr(s.length() - terminator.length(), terminator.length());
    if (ending == terminator)
        return true;
    return false;
}


struct XGenPatch
{
    std::string scene;
    std::string palette;
    std::string description;
    std::string patch;
    std::string geom;
    std::string file;
    bool hasGeoCache;
};

void buildXGenBasePaths(const std::string& xgenRoot, std::string& out_basePath, std::string& out_sceneBasePath)
{
    out_basePath = xgenRoot;
    platformizePath(out_basePath, true); // XGen expects forward slashes
    // Remove trailing slash (temporarily)
    out_basePath = stripFromEnd(out_basePath, "/");
    // Remove xgen or scenes directory if the user specified it (we need the actual project root)
    out_basePath = stripFromEnd(out_basePath, "/xgen");
    out_basePath = stripFromEnd(out_basePath, "/scenes");
    // Put trailing slash back on
    out_basePath += "/";
    // Scenes directory
    out_sceneBasePath = out_basePath + "scenes/";
}

void buildXGenBasePathsFromFile(const std::string& xgenFile, std::string& out_basePath, std::string& out_sceneBasePath)
{
    std::string xgenRoot = xgenFile;
    platformizePath(xgenRoot, true);
    size_t pos = xgenRoot.rfind('/');
    if (pos != std::string::npos)
        xgenRoot.erase(pos, xgenRoot.length() - pos);

    buildXGenBasePaths(xgenRoot, out_basePath, out_sceneBasePath);
    out_sceneBasePath = xgenRoot + "/"; // Override and use same directory as the .xgen file
}

bool reconstructSceneAndPalette(const std::string& filename, std::string& out_scene, std::string& out_paletteWithNamespace)
{
    // Reconstruct palette with namespace from the filename
    std::vector<std::string> filenameParts;
    std::string filenameOnly = filename;
    platformizePath(filenameOnly, true);
    size_t pos = filenameOnly.rfind('/');
    if (pos != std::string::npos)
        filenameOnly.erase(0, pos + 1);
    size_t startPos = 0;
    for (size_t i = 0; i < filenameOnly.length(); ++i)
    {
        // See if the extention is ".xgen"; if so, we're done
        if (filenameOnly[i] == '.' && i + 5 == filenameOnly.length() && i > 0)
        {
            filenameParts.push_back(filenameOnly.substr(startPos, i - startPos));
            break;
        }
        if (filenameOnly[i] == '_' && i + 1 < filenameOnly.length() && filenameOnly[i + 1] == '_')
        {
            filenameParts.push_back(filenameOnly.substr(startPos, i - startPos));
            startPos = i + 2;
            ++i;
        }
    }
    if (filenameParts.size() <= 1)
        return false;
    out_paletteWithNamespace.clear();
    for (size_t i = 1; i < filenameParts.size(); ++i)
    {
        out_paletteWithNamespace += filenameParts[i];
        if (i + 1 < filenameParts.size())
            out_paletteWithNamespace += ":";
    }
    out_scene = filenameParts[0];
    return true;
}

void buildXGenPatches(std::vector<XGenPatch>& out_xgpatches, const std::string& xgenRoot, const std::string& xgenFile, const std::string& scene, const std::string& palette, const std::string& description, const std::vector<std::string>& patches)
{
    std::string basePath, sceneBasePath;
    if (!xgenFile.empty())
    {
        buildXGenBasePathsFromFile(xgenFile, basePath, sceneBasePath);
    }
    else
    {
        buildXGenBasePaths(xgenRoot, basePath, sceneBasePath);
    }

    std::string scenePath = sceneBasePath + scene;
    std::string geomPath = scenePath + "__" + palette + ".abc";
    bool hasGeoCache = fileExists(geomPath);
    if (!hasGeoCache)
    {
        // Replace Maya namespace markers with __ns__
        for (size_t pos = 2; (pos = geomPath.find(':', pos)) != std::string::npos; pos += 6)
            geomPath.replace(pos, 1, "__ns__");
        hasGeoCache = fileExists(geomPath);
    }
    std::string paletteStr = palette;
    std::string paletteFileStr = paletteStr;
    // Replace Maya namespace markers with __ in the filename
    for (size_t pos = 0; (pos = paletteFileStr.find(':', pos)) != std::string::npos; pos += 2)
        paletteFileStr.replace(pos, 1, "__");
    std::string paletteSansNamespace = paletteStr;
    std::string descSansNamespace = description;
    if (hasGeoCache)
    {
        // XGen expects palette, patch and description without namespaces
        size_t pos = paletteSansNamespace.rfind(':');
        if (pos != std::string::npos)
            paletteSansNamespace.erase(0, pos + 1);

        pos = descSansNamespace.rfind(':');
        if (pos != std::string::npos)
            descSansNamespace.erase(0, pos + 1);
    }
    XGenPatch xgp;
    xgp.scene = scene;
    xgp.description = descSansNamespace;
    xgp.hasGeoCache = hasGeoCache;
    if (hasGeoCache)
        xgp.geom = geomPath;
    else
        xgp.geom = scenePath + "__" + paletteSansNamespace + ".abc";
    xgp.palette = paletteSansNamespace;
    if (!xgenFile.empty())
    {
        xgp.file = xgenFile;
        platformizePath(xgp.file, true);
    }
    else
        xgp.file = scenePath + "__" + paletteFileStr + ".xgen";

    // Create one sub-op for each patch (one procedural per patch)
    for (size_t i = 0; i < patches.size(); ++i)
    {
        std::string patchSansNamespace = patches[i];
        if (hasGeoCache)
        {
            // XGen expects palette, patch and description without namespaces
            size_t pos = patchSansNamespace.rfind(':');
            if (pos != std::string::npos)
                patchSansNamespace.erase(0, pos + 1);
        }
        XGenPatch xgpatch = xgp;
        xgpatch.patch = patchSansNamespace;
        out_xgpatches.push_back(xgpatch);
    }
}

struct FoundPatch
{
    std::string palette;
    std::string description;
    std::vector<std::string> patches;
};

void extractXGenPatches(std::vector<XGenPatch>& out_xgpatches, const std::string& xgenRoot, const std::string& xgenFile, const std::string& scene, const std::string& palette, const std::string& description, const std::vector<std::string>& patches)
{
    std::string basePath, sceneBasePath;
    std::vector<std::string> xgenFiles;
    if (!xgenFile.empty())
    {
        buildXGenBasePathsFromFile(xgenFile, basePath, sceneBasePath);
        xgenFiles.push_back(xgenFile);
    }
    else
    {
        buildXGenBasePaths(xgenRoot, basePath, sceneBasePath);
        xgenFiles = findXGenFiles(sceneBasePath, scene);
    }

    const size_t lineSize = 1024 * 1024 * 10;
    char *line = new char[lineSize];
    char field1[1024];
    char field2[1024];
    line[0] = field1[0] = field2[0] = 0;

    // For each candidate xgen file...
    std::vector<FoundPatch> foundPatches;
    for (size_t xfi = 0; xfi < xgenFiles.size(); ++xfi)
    {
        // Reconstruct palette with namespace from the filename
        std::string paletteWithNamespace, expectedScene;
        if (!reconstructSceneAndPalette(xgenFiles[xfi], expectedScene, paletteWithNamespace))
            continue;
        if (!scene.empty() && scene != expectedScene)
            continue;

        std::ifstream stream(xgenFiles[xfi].c_str());
        // Scan lines looking for a few tags
        std::vector<std::string> descriptions;
        bool inPalette = false, inDescription = false, inPatch = false;
        std::string currentPatchDescription;
        bool currentPatchDescriptionIncluded = false;
        std::string foundPalette;
        while (stream.good())
        {
            stream.getline(line, lineSize - 1);
            if (inPalette || inDescription)
            {
#ifdef _WIN32
                int fields = sscanf_s(line, " %s %s", field1, (unsigned int)sizeof(field1), field2, (unsigned int)sizeof(field2));
#else
                int fields = sscanf(line, " %s %s", field1, field2);
#endif
                if (fields == 1 && std::string("endAttrs") == field1)
                    inPalette = inDescription = false;
                else if (fields == 2 && std::string("name") == field1)
                {
                    if (inDescription && (description.empty() || endsWith(description, field2)))
                        descriptions.push_back(field2);
                    else if (inPalette && (palette.empty() || endsWith(palette, field2)))
                    {
                        foundPalette = field2;
                        if (!endsWith(paletteWithNamespace, foundPalette))
                        {
                            // The palette doesn't actually match the expected value from the name!
                            foundPalette.clear();
                        }
                    }
                }
            }
            else if (inPatch)
            {
                if (!currentPatchDescriptionIncluded || foundPalette.empty())
                    continue;
#ifdef _WIN32
                int fields = sscanf_s(line, " %s %s", field1, (unsigned int)sizeof(field1), field2, (unsigned int)sizeof(field2));
#else
                int fields = sscanf(line, " %s %s", field1, field2);
#endif
                if (fields == 1 && std::string("endObject") == field1)
                {
                    inPatch = false;
                }
                else if (fields == 1 && std::string("endPatches") == field1)
                {
                    currentPatchDescription.clear();
                    currentPatchDescriptionIncluded = false;
                    inPatch = false;
                }
                else if (fields == 2 && std::string("name") == field1)
                {
                    std::string thisPatch = field2;
                    bool includePatch = false;
                    if (patches.empty())
                        includePatch = true;
                    else
                    {
                        // Search the patches list to see if we find a match
                        for (size_t pi = 0; pi < patches.size(); ++pi)
                        {
                            if (endsWith(patches[pi], thisPatch))
                            {
                                includePatch = true;
                                break;
                            }
                        }
                    }
                    if (includePatch)
                    {
                        bool addNew = true;
                        for (size_t fpi = 0; fpi < foundPatches.size(); ++fpi)
                        {
                            if (foundPatches[fpi].description == currentPatchDescription &&
                                foundPatches[fpi].palette == paletteWithNamespace)
                            {
                                addNew = false;
                                foundPatches[fpi].patches.push_back(thisPatch);
                            }
                        }
                        if (addNew)
                        {
                            foundPatches.push_back(FoundPatch());
                            foundPatches.back().description = currentPatchDescription;
                            foundPatches.back().palette = paletteWithNamespace;
                            foundPatches.back().patches.push_back(thisPatch);
                        }
                    }
                }
            }
            else if (!strncmp(line, "Palette", 7))
            {
                currentPatchDescription.clear();
                currentPatchDescriptionIncluded = false;
                inDescription = inPatch = false;
                inPalette = true;
            }
            else if (!strncmp(line, "Description", 11))
            {
                currentPatchDescription.clear();
                currentPatchDescriptionIncluded = false;
                inPalette = inPatch = false;
                inDescription = true;
            }
            else if (!strncmp(line, "Patches", 7))
            {
                field1[0] = 0;
#ifdef _WIN32
                int fields = sscanf_s(line, "%*s %s", field1, (unsigned int)sizeof(field1));
#else
                int fields = sscanf(line, "%*s %s", field1);
#endif
                if (fields == 1)
                {
                    currentPatchDescription = field1;
                    currentPatchDescriptionIncluded = false;
                    for (size_t di = 0; di < descriptions.size(); ++di)
                    {
                        if (currentPatchDescription == descriptions[di])
                        {
                            currentPatchDescriptionIncluded = true;
                            break;
                        }
                    }
                }
            }
            else if (!strncmp(line, "Patch", 5)) // Order is important, check for 'Patches' first
                inPatch = true;
        }

        // Submit what we have so far for this scene/palette combo
        for (size_t i = 0; i < foundPatches.size(); ++i)
            buildXGenPatches(out_xgpatches, xgenRoot, xgenFile, expectedScene, foundPatches[i].palette, foundPatches[i].description, foundPatches[i].patches);
        foundPatches.clear();
    }
    delete[] line;
}


class ArnoldXGenOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface& interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface& interface)
    {
        FnAttribute::StringAttribute mayaPathAttr      = interface.getOpArg("maya_path");
        FnAttribute::StringAttribute mtoaPathAttr      = interface.getOpArg("mtoa_path");
        FnAttribute::StringAttribute xgenLocAttr       = interface.getOpArg("xgen_location");
        FnAttribute::IntAttribute    verbosityAttr     = interface.getOpArg("verbose");
        FnAttribute::IntAttribute    frameAttr         = interface.getOpArg("frame");
        FnAttribute::StringAttribute paletteAttr       = interface.getOpArg("palette"); // XGen collection
        FnAttribute::StringAttribute patchAttr         = interface.getOpArg("patch");
        FnAttribute::StringAttribute descriptionAttr   = interface.getOpArg("description");
        FnAttribute::StringAttribute nameSchemeAttr    = interface.getOpArg("name_scheme");
        FnAttribute::FloatAttribute  fpsAttr           = interface.getOpArg("fps");
        FnAttribute::FloatAttribute  samplesAttr       = interface.getOpArg("samples");
        FnAttribute::FloatAttribute  minPixelWidthAttr = interface.getOpArg("min_pixel_width");
        FnAttribute::IntAttribute    modeAttr          = interface.getOpArg("mode");
        FnAttribute::StringAttribute cameraAttr        = interface.getOpArg("camera");

        std::string mtoaPath = mtoaPathAttr.getValue();
        if (mtoaPath.empty())
            mtoaPath = getenvHelper("MTOA_PATH");
        platformizePath(mtoaPath, true);
        if (!mtoaPath.empty() && mtoaPath[mtoaPath.length() - 1] != '/')
            mtoaPath += "/";
        if (mtoaPath.empty())
        {
            Foundry::Katana::ReportError(interface, "Invalid \"mtoa_path\" attribute and MTOA_PATH environment variable not set.");
            return;
        }

        std::string mayaPath = mayaPathAttr.getValue();
        if (mayaPath.empty())
            mayaPath = getenvHelper("MAYA_PATH");
        platformizePath(mayaPath);
        if (mayaPath.empty())
        {
            Foundry::Katana::ReportError(interface, "Invalid \"maya_path\" attribute and MAYA_PATH environment variable not set.");
            return;
        }

        FnAttribute::StringAttribute leafAttr = interface.getOpArg("leaf");
        if (!leafAttr.isValid() || leafAttr.getValue() != "patch")
        {
            interface.setAttr("type", FnAttribute::StringAttribute("group"));

            FnAttribute::StringAttribute sceneAttr    = interface.getOpArg("scene");
            FnAttribute::StringAttribute xgenRootAttr = interface.getOpArg("xgen_root");
            FnAttribute::StringAttribute xgenFileAttr = interface.getOpArg("xgen_file");
            FnAttribute::IntAttribute    specAttr     = interface.getOpArg("specification");

            std::vector<std::string> stdpatches;
            if (patchAttr.isValid())
            {
                FnAttribute::StringConstVector patches = patchAttr.getNearestSample(0);
                for (size_t i = 0; i < patches.size(); ++i)
                    stdpatches.push_back(patches[i]);
            }

            std::vector<XGenPatch> xgpatches;
            bool singleXGenFile = specAttr.isValid() && specAttr.getValue() == 0;
            if (singleXGenFile)
            {
                // Build from a single .xgen file
                extractXGenPatches(xgpatches, std::string(), xgenFileAttr.getValue(), std::string(), std::string(), descriptionAttr.getValue(), stdpatches);
            }
            else
            {
                // Go hunting for .xgen files and extra info from potentially multiple files
                extractXGenPatches(xgpatches, xgenRootAttr.getValue(), std::string(), sceneAttr.getValue(), paletteAttr.getValue(), descriptionAttr.getValue(), stdpatches);
            }

            leafAttr = FnAttribute::StringAttribute("patch");
            for (size_t i = 0; i < xgpatches.size(); ++i)
            {
                FnAttribute::GroupBuilder gb;
                gb.set("maya_path",       mayaPathAttr);
                gb.set("mtoa_path",       mtoaPathAttr);
                gb.set("xgen_location",   xgenLocAttr);
                gb.set("verbose",         verbosityAttr);
                gb.set("frame",           frameAttr);
                gb.set("file",            FnAttribute::StringAttribute(xgpatches[i].file));
                gb.set("palette",         FnAttribute::StringAttribute(xgpatches[i].palette));
                gb.set("patch",           FnAttribute::StringAttribute(xgpatches[i].patch));
                gb.set("description",     FnAttribute::StringAttribute(xgpatches[i].description));
                gb.set("geom",            FnAttribute::StringAttribute(xgpatches[i].geom));
                gb.set("fps",             fpsAttr);
                gb.set("samples",         samplesAttr);
                gb.set("min_pixel_width", minPixelWidthAttr);
                gb.set("mode",            modeAttr);
                gb.set("camera",          cameraAttr);
                gb.set("leaf",            leafAttr);

                std::string nameScheme = nameSchemeAttr.getValue();
                if (nameScheme.empty())
                    nameScheme = "<scene>_<collection>_<description>_<patch>";
                std::string childName;
                for (size_t ni = 0; ni < nameScheme.length(); ++ni)
                {
                    if (nameScheme[ni] == '<')
                    {
                        size_t endTagLoc = nameScheme.find('>', ni + 1);
                        if (endTagLoc != std::string::npos)
                        {
                            std::string tag = nameScheme.substr(ni + 1, endTagLoc - ni - 1);
                            if (tag == "scene")
                            {
                                childName += xgpatches[i].scene;
                                ni = endTagLoc;
                                continue;
                            }
                            else if (tag == "collection")
                            {
                                childName += xgpatches[i].palette;
                                ni = endTagLoc;
                                continue;
                            }
                            else if (tag == "description")
                            {
                                childName += xgpatches[i].description;
                                ni = endTagLoc;
                                continue;
                            }
                            else if (tag == "patch")
                            {
                                childName += xgpatches[i].patch;
                                ni = endTagLoc;
                                continue;
                            }
                        }
                    }
                    childName += nameScheme[ni];
                }

                interface.createChild(childName, std::string(), gb.build());
            }
            return;
        }

        interface.setAttr("type", FnAttribute::StringAttribute("renderer procedural"));

        FnAttribute::StringAttribute fileAttr = interface.getOpArg("file");
        FnAttribute::StringAttribute geomAttr = interface.getOpArg("geom");

#ifdef _WIN32
        std::string dsoext = ".dll";
#else
        std::string dsoext = ".so";
#endif
        std::string procPath = mtoaPath + "procedurals/xgen_procedural" + dsoext;
        platformizePath(procPath); // Arnold can use native path separators
        if (!fileExists(procPath))
        {
            procPath = mtoaPath + "shaders/xgen_procedural" + dsoext;
            platformizePath(procPath);
        }

        std::ostringstream samplesStream;
        float initialCameraSample = 0.0f;
        if (samplesAttr.getNumberOfValues() > 0)
        {
            FnAttribute::FloatConstVector samples = samplesAttr.getNearestSample(0.0f);
            if (samples.size())
                initialCameraSample = samples[0];
            for (size_t i = 0; i < samples.size(); ++i)
            {
                samplesStream << samples[i] << ' ';
            }
        }
        else
        {
            samplesStream << "0.0";
        }
        samplesStream.flush();

        /* // Reference from MtoA for unit conversions, when we get to it
        static std::map<std::string, std::pair<std::string,float> > s_mapUnitsConv;
        if( s_mapUnitsConv.empty() )
        {
           s_mapUnitsConv["in"] = std::pair<std::string,float>( "2.54", 2.54f );
           s_mapUnitsConv["ft"] = std::pair<std::string,float>( "30.48", 30.48f );
           s_mapUnitsConv["yd"] = std::pair<std::string,float>( "91.44", 91.44f );
           s_mapUnitsConv["mi"] = std::pair<std::string,float>( "160934.4", 160934.4f );
           s_mapUnitsConv["mm"] = std::pair<std::string,float>( "0.1", 0.1f );
           s_mapUnitsConv["km"] = std::pair<std::string,float>( "100000.0", 100000.f );
           s_mapUnitsConv["m"]  = std::pair<std::string,float>( "100.0", 100.f );
           s_mapUnitsConv["dm"] = std::pair<std::string,float>( "10.0", 10.f );
        }

        std::string factor = "1";
        std::map<std::string, std::pair<std::string,float> >::const_iterator it = s_mapUnitsConv.find( strCurrentUnits );
        if( it!=s_mapUnitsConv.end() )
        {
           factor = it->second.first;
           fUnitConvFactor = it->second.second;
        }
        strUnitConvMat = " -world "+factor+";0;0;0;0;"+factor+";0;0;0;0;"+factor+";0;0;0;0;1";
        */

        // Set up 'data' string arg
        std::ostringstream dataArgsStream;
        dataArgsStream << "-debug " << verbosityAttr.getValue(1)
                       << " -warning " << verbosityAttr.getValue(1)
                       << " -stats " << verbosityAttr.getValue(1);
        dataArgsStream << " -frame " << frameAttr.getValue();
        dataArgsStream << " -fps " << fpsAttr.getValue();
        dataArgsStream << " -motionSamplesLookup " << samplesStream.str();
        dataArgsStream << " -motionSamplesPlacement " << samplesStream.str();
        dataArgsStream << " -file " << fileAttr.getValue();
        dataArgsStream << " -palette " << paletteAttr.getValue();
        dataArgsStream << " -patch " << patchAttr.getValue();
        dataArgsStream << " -description " << descriptionAttr.getValue();
        dataArgsStream << " -geom " << geomAttr.getValue();
        // TODO: world scale factor due to units?
        dataArgsStream << " -world 1;0;0;0;0;1;0;0;0;0;1;0;0;0;0;1";
        dataArgsStream.flush();

        std::string camLoc = cameraAttr.getValue();
        if (interface.doesLocationExist(camLoc))
        {
            FnAttribute::FloatAttribute  fovAttr     = interface.getAttr("geometry.fov", camLoc);
            FnAttribute::FloatAttribute  windowLAttr = interface.getAttr("geometry.left", camLoc);
            FnAttribute::FloatAttribute  windowRAttr = interface.getAttr("geometry.right", camLoc);
            FnAttribute::FloatAttribute  windowBAttr = interface.getAttr("geometry.bottom", camLoc);
            FnAttribute::FloatAttribute  windowTAttr = interface.getAttr("geometry.top", camLoc);
            FnAttribute::StringAttribute projAttr    = interface.getAttr("geometry.projection", camLoc);

            float camHRatio = (windowRAttr.getValue(1.0f, false) - windowLAttr.getValue(-1.0, false)) * 0.5f;
            float camVRatio = (windowTAttr.getValue(1.0f, false) - windowBAttr.getValue(-1.0, false)) * 0.5f;
            float camHFOV   = fovAttr.getValue(70.0f, false) * camHRatio;
            float camVFOV   = fovAttr.getValue(70.0f, false) * camVRatio;
            bool  ortho     = projAttr.getValue("perspective", false) == "perspective" ? false : true;
            float camRatio  = camHRatio / camVRatio;

            FnAttribute::GroupAttribute xformGroupAttr = Foundry::Katana::GetGlobalXFormGroup(interface, camLoc);
            FnAttribute::DoubleAttribute matrixAttr = FnGeolibServices::FnXFormUtil::CalcTransformMatrixAtTime(xformGroupAttr, initialCameraSample).first;
            Imath::M44d matrix((const double(*)[4])matrixAttr.getNearestSample(0.0f).data());
            Imath::V3d worldPos, localPos(0);
            matrix.multVecMatrix(localPos, worldPos);
            Imath::M44d invMatrix = matrix.inverse();

            {
                std::ostringstream stream;
                stream << (ortho ? "true" : "false") << ',' << worldPos.x << ',' << worldPos.y << ',' << worldPos.z << std::flush;
                interface.setAttr("rendererProcedural.args.irRenderCam", FnAttribute::StringAttribute(stream.str()));
            }
            {
                std::ostringstream stream;
                stream << camHFOV << ',' << camVFOV << std::flush;
                interface.setAttr("rendererProcedural.args.irRenderCamFOV", FnAttribute::StringAttribute(stream.str()));
            }
            {
                std::ostringstream stream;
                stream << camRatio << std::flush;
                interface.setAttr("rendererProcedural.args.irRenderCamRatio", FnAttribute::StringAttribute(stream.str()));
            }
            {
                std::ostringstream stream;
                stream << invMatrix.getValue()[0];
                for (int i = 1; i < 16; ++i)
                    stream << ',' << invMatrix.getValue()[i];
                stream.flush();
                interface.setAttr("rendererProcedural.args.irRenderCamXform", FnAttribute::StringAttribute(stream.str()));
            }
        }
        else
        {
            interface.setAttr("rendererProcedural.args.irRenderCam",      FnAttribute::StringAttribute("false,0.0,0.0,0.0"));
            interface.setAttr("rendererProcedural.args.irRenderCamFOV",   FnAttribute::StringAttribute("0.0,0.0"));
            interface.setAttr("rendererProcedural.args.irRenderCamRatio", FnAttribute::StringAttribute("0.0"));
            interface.setAttr("rendererProcedural.args.irRenderCamXform", FnAttribute::StringAttribute("0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0"));
        }

        // Specify additional libraries to pre-load which are required by xgen_procedural
        std::vector<std::string> libs;
        std::vector<std::string> libpaths;
#ifdef _WIN32
        // On Windows we can only extend the module searchpath; preloading individual DLLs doesn't work
        libpaths.push_back("bin");
        libpaths.push_back("plug-ins\\xgen\\bin");

        if (!mayaPath.empty() && mayaPath.back() != '\\')
            mayaPath += "\\";
#else
        // On Linux we can preload specific SOs into the global symbol space,
        // and subsequent Arnold plugins (xgen_procedural) will use them automatically.
        libs.push_back("lib/libtbb.so.2");
        libs.push_back("lib/libtbbmalloc.so.2");
        libs.push_back("lib/libPtex.so");
        libs.push_back("plug-ins/xgen/lib/libAdskSubEngine.so");
        libs.push_back("plug-ins/xgen/lib/libAdskFabricMath.so");
        libs.push_back("plug-ins/xgen/lib/libAdskFabricGeom.so");
        libs.push_back("plug-ins/xgen/lib/libAdskCaf.so");
        libs.push_back("plug-ins/xgen/lib/libAdskPartio.so");
        libs.push_back("plug-ins/xgen/lib/libAdskGLee.so");
        libs.push_back("plug-ins/xgen/lib/libAdskXGen.so");
        libs.push_back("plug-ins/xgen/lib/libAdskSeExpr.so");

        if (!mayaPath.empty() && mayaPath[mayaPath.length() - 1] != '/')
            mayaPath += "/";
#endif
        for (size_t i = 0; i < libs.size(); ++i)
        {
            libs[i] = mayaPath + libs[i];
        }
        for (size_t i = 0; i < libpaths.size(); ++i)
        {
            libpaths[i] = mayaPath + libpaths[i];
        }

        // Specify additional environment variables that are needed by xgen_procedural
        std::vector<std::string> envVars;
        std::string xgenLocation = xgenLocAttr.getValue();
        if (xgenLocation.empty())
        {
            // Try to get XGEN_LOCATION from the environment
            xgenLocation = getenvHelper("XGEN_LOCATION");
            if (xgenLocation.empty())
            {
                // Wasn't even found there, build it from MAYA_PATH
                xgenLocation = mayaPath;
                platformizePath(xgenLocation, true);
                if (!xgenLocation.empty() && xgenLocation[xgenLocation.length() - 1] != '/')
                    xgenLocation += "/";
                xgenLocation += "plug-ins/xgen/";
                envVars.push_back("XGEN_LOCATION");
                envVars.push_back(xgenLocation);
            }
            else
            {
                // Ensure paths are the way xgen expects, which are generally forward slashes
                platformizePath(xgenLocation, true);
                envVars.push_back("XGEN_LOCATION");
                envVars.push_back(xgenLocation);
            }
        }
        else
        {
            // Use user-defined XGEN_LOCATION
            platformizePath(xgenLocation, true);
            envVars.push_back("XGEN_LOCATION");
            envVars.push_back(xgenLocation);
        }

        // Set final proc parameters

        interface.setAttr("rendererProcedural.procedural",         FnAttribute::StringAttribute(procPath));
        // Suppress Katana outputting automatic args for the camera
        interface.setAttr("rendererProcedural.includeCameraInfo",  FnAttribute::StringAttribute("None"));
        // classic, scenegraphAttr, typedArguments; default to typedArguments
        interface.setAttr("rendererProcedural.args.__outputStyle", FnAttribute::StringAttribute("typedArguments"));
        // Don't worry about bounds, let the procedural take care of that for its children
        interface.setAttr("rendererProcedural.useInfiniteBounds",  FnAttribute::IntAttribute(1));
        if (!libs.empty())
            interface.setAttr("rendererProcedural.libraries", FnAttribute::StringAttribute(libs));
        if (!libpaths.empty())
            interface.setAttr("rendererProcedural.libpaths",  FnAttribute::StringAttribute(libpaths));
        if (!envVars.empty())
            interface.setAttr("rendererProcedural.envvars",   FnAttribute::StringAttribute(envVars));

        interface.setAttr("rendererProcedural.args.data",               FnAttribute::StringAttribute(dataArgsStream.str()));
        interface.setAttr("rendererProcedural.args.ai_min_pixel_width", minPixelWidthAttr);
        interface.setAttr("rendererProcedural.args.time_samples",       samplesAttr);
        interface.setAttr("rendererProcedural.args.xgen_renderMethod",  FnAttribute::StringAttribute("3")); // 1: live, 3: batch
        interface.setAttr("rendererProcedural.args.ai_mode",            modeAttr);
        interface.setAttr("rendererProcedural.args.xgen_shader",        FnAttribute::StringAttribute("")); // value doesn't matter, will get cloned from shaders

        // These extra params are interpreted by the procedural args translator
        // to set the DSO argument types, declarations and values correctly
        std::vector<std::string> shaderHint;
        // The xgen_shader array needs to copy from the regularly assigned shaders,
        // and needs to declare it as a pointer array
        shaderHint.push_back("clone");
        shaderHint.push_back("shader");
        shaderHint.push_back("type");
        shaderHint.push_back("pointer");
        shaderHint.push_back("array");
        shaderHint.push_back("true");
        FnAttribute::StringAttribute shaderHintAttr(shaderHint);
        interface.setAttr("rendererProcedural.args.arnold_hint__xgen_shader", shaderHintAttr);

        interface.stopChildTraversal();
    }
};


DEFINE_GEOLIBOP_PLUGIN(ArnoldXGenOp)


} // anonymous


void registerPlugins()
{
    REGISTER_PLUGIN(ArnoldXGenOp, "ArnoldXGen", 0, 1);
}
