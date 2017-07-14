#include <fnmatch.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>

#include <FnGeolib/util/Path.h>

#include <pystring/pystring.h>

FNGEOLIBUTIL_NAMESPACE_ENTER
{

namespace Path
{
std::string StripTrailingSlash(const std::string & locationPath)
{
    if(locationPath.empty()) return locationPath;
    if(locationPath[locationPath.size()-1] == '/')
    {
        return locationPath.substr(0, locationPath.size()-1);
    }
    else
    {
        return locationPath;
    }
}

std::string AssureTrailingSlash(const std::string & locationPath)
{
    if(locationPath.empty()) return "/";

    if(locationPath[locationPath.size()-1] == '/')
    {
        return locationPath;
    }
    else
    {
        return locationPath + "/";
    }
}

std::string NormalizeAbsPath(const std::string & locationPath)
{
    return StripTrailingSlash(
        pystring::os::path::normpath_posix(locationPath));
}

std::string GetLocationParent(const std::string & locationPath)
{
    size_t index = locationPath.find_last_of("/");
    if(index == std::string::npos) return "";
    return locationPath.substr(0, index);

    /*
    std::vector<std::string> result;
    pystring::rsplit(locationPath, result, "/", 1);
    if (result.size() <= 1) return ""; // workaround for pystring bug
    std::string parent = result[0];
    */
}

std::string GetLeafName(const std::string & locationPath)
{
    size_t index = locationPath.find_last_of("/");
    if(index == std::string::npos) return "";
    return locationPath.substr(index+1, locationPath.size()-1);

    /*
    std::vector<std::string> result;
    pystring::rsplit(locationPath, result, "/", 1);
    // workaround for pystring bug
    if (result.size() == 0) return "";
    if (result.size() == 1) return pystring::slice(locationPath, 1);
    return result[result.size()-1];
    */
}

void GetLeafAndParent(std::string & parent,
    std::string & leaf,
    const std::string & locationPath)
{
    size_t index = locationPath.find_last_of("/");
    if(index == std::string::npos)
    {
        parent = "";
        leaf = "";
    }
    else
    {
        parent = locationPath.substr(0, index);
        leaf = locationPath.substr(index+1, locationPath.size()-1);
    }
}

// TODO: This can be hugely optimized

void GetLocationStack(std::vector<std::string> & returnStack,
    const std::string & locationPath, const std::string & rootPath)
{
    returnStack.clear();

    if (rootPath.empty())
    {
        std::vector<std::string> parts;
        pystring::split(pystring::strip(locationPath, "/"), parts, "/");
        for (unsigned int i=0; i<parts.size(); ++i)
        {
            returnStack.push_back(
                "/" + pystring::join("/", std::vector<std::string>(parts.begin(), parts.begin()+i+1)));
        }
    }
    else
    {
        if (locationPath == rootPath)
        {
            returnStack.push_back(locationPath);
        }
        else
        {
            returnStack.push_back(rootPath);
            std::string normRootPath = "/"+pystring::strip(rootPath, "/"); //ensure normalized
            if (Path::IsAncestorOrEqual(normRootPath, locationPath))
            {
                std::string rel = pystring::slice(locationPath, (int)normRootPath.size());
                std::vector<std::string> parts;
                pystring::split(pystring::strip(rel, "/"), parts, "/");
                for (unsigned int i=0; i<parts.size(); ++i)
                {
                    returnStack.push_back(
                        normRootPath + "/" + pystring::join("/", std::vector<std::string>(parts.begin(), parts.begin()+i+1)));
                }
            }
        }
    }
}

bool IsAncestorOrEqual(const std::string & locA, const std::string & locB)
{
    return pystring::startswith(
        AssureTrailingSlash(locB),
        AssureTrailingSlash(locA));
}

bool IsAncestor(const std::string & locA, const std::string & locB)
{
    std::string stripA = StripTrailingSlash(locA);
    std::string stripB = StripTrailingSlash(locB);
    if (stripA == stripB) return false;
    return pystring::startswith(stripB + "/", stripA + "/");
}

std::string Join(const std::string & locA, const std::string & locB)
{
    return StripTrailingSlash(locA)  + "/" + locB;
}

bool IsRelativePath(const std::string & path)
{
    return !(pystring::os::path::isabs_posix(path));
}

std::string RelativeToAbsPath(const std::string & rootPath, const std::string & path)
{
    if(pystring::os::path::isabs_posix(path))
    {
        return path;
    }
    else
    {
        return pystring::os::path::normpath_posix(
                pystring::os::path::join_posix(rootPath, path));
    }
}

// TODO: This can be hugely optimized

std::string NormalizedRelativePath(const std::string & rootpath_,
    const std::string & path)
{
    std::string rootpath = StripTrailingSlash(rootpath_);
    std::string abspath = pystring::rstrip(RelativeToAbsPath(rootpath, path));

    if(!IsAncestorOrEqual(rootpath, abspath))
    {
        std::ostringstream err;
        err << "The rootpath " << rootpath_;
        err << " is not an ancestor or equal to " << abspath;
        throw std::runtime_error(err.str().c_str());
    }

    std::string result = pystring::replace(abspath, rootpath, "", 1);
    return pystring::lstrip(result, "/");
}

std::string RelativePath(const std::string & rootPath,
        const std::string & path)
{
    std::string absRootPath = NormalizeAbsPath(rootPath);
    std::string absPath = NormalizeAbsPath(path);

    std::vector<std::string> rootPathSplit;
    std::vector<std::string> pathSplit;
    pystring::split(absRootPath, rootPathSplit, "/");
    pystring::split(absPath, pathSplit, "/");

    // find common prefix
    int commonIndex = -1;
    for (std::vector<std::string>::const_iterator
        I=pathSplit.begin(), E=pathSplit.end(),
        rI=rootPathSplit.begin(), rE=rootPathSplit.end();
        I!=E && rI!=rE;
        ++I, ++rI)
    {
        if (*I == *rI)
        {
            ++commonIndex;
        }
        else
        {
            break;
        }
    }

    // build relative path
    std::vector<std::string> relPathSplit;
    relPathSplit.reserve(((int)rootPathSplit.size()-commonIndex-1) +
                         ((int)pathSplit.size()-commonIndex+1));
    for (int i=0; i<(int)rootPathSplit.size()-commonIndex-1; ++i)
    {
        relPathSplit.push_back("..");
    }
    for (int i=commonIndex+1; i<(int)pathSplit.size(); ++i)
    {
        relPathSplit.push_back(pathSplit[i]);
    }
    return pystring::join("/", relPathSplit);
}

void ExactMatch(FnMatchInfo & matchInfo,
                 const std::string & testpath,
                 const std::string & pattern)
{
    if(testpath == pattern)
    {
        matchInfo.match = true;
        matchInfo.canMatchChildren = false;
    }
    else
    {
        matchInfo.match = false;
        matchInfo.canMatchChildren = IsAncestor(testpath, pattern);
    }
}

void FnMatch(FnMatchInfo & matchInfo,
             const std::string & testpath,
             const std::string & pattern)
{
    std::vector<std::string> pathparts;
    std::vector<std::string> patternparts;

    pystring::split(pystring::strip(testpath, "/"), pathparts, "/");
    pystring::split(pystring::strip(pattern, "/"), patternparts, "/");

    // If the pattern is shorter than test path,
    // there is no way either this location of any of the children can match
    // (no matter what the pattern is)
    //
    // Example:
    //     pattern /root/world/*
    //     test    /root/world/a/b
    //
    // In this case, b doesnt match the pattern, nor can its children

    if(patternparts.size() < pathparts.size())
    {
        matchInfo.match = false;
        matchInfo.canMatchChildren = false;
        return;
    }

    int commonLength = (int) std::min(pathparts.size(), patternparts.size());

    // Start from the right side, and walk backwards, to maximize the chance
    // of an early mismatch
    for(int i=commonLength-1; i>=0; --i)
    {
        if(patternparts[i] == pathparts[i]) continue;

        int result = fnmatch(patternparts[i].c_str(), pathparts[i].c_str(), 0);
        bool localpartmatch = (result == 0);
        if(!localpartmatch)
        {
            // Early exit, no match is possible
            matchInfo.match = false;
            matchInfo.canMatchChildren = false;
            return;
        }
    }

    // All the overlapping parts match
    if(patternparts.size() == pathparts.size())
    {
        matchInfo.match = true;
        matchInfo.canMatchChildren = false;
    }
    else if(patternparts.size() > pathparts.size())
    {
        // If we still have chunks of pattern remaining,
        // we dont match. But our children may

        matchInfo.match = false;
        matchInfo.canMatchChildren = true;
    }
    else
    {
        // This case can't happen due to the early exit at the top.
        throw std::runtime_error("FnMatch internal error");
    }
}

std::string MakeUniqueName(const std::string & baseName,
                           const std::set<std::string> & existingNames)
{
    std::string name = baseName;
    int count = 1;
    while (existingNames.find(name) != existingNames.end())
    {
        std::ostringstream nameStr;
        nameStr << baseName << count;
        name = nameStr.str();
        ++count;
    }

    return name;
}

std::string MakeSafeIdentifier(const std::string &identifier)
{
    if (identifier.empty()) return std::string("_");

    std::string result(identifier);
    for (std::string::iterator i=result.begin(), e=result.end();
         i!=e; ++i)
    {
        if (!std::isalnum(*i))
        {
            *i = '_';
        }
    }

    if (!std::isalpha(result[0]) && result[0] != '_')
    {
        return std::string("_") + result;
    }

    return result;
}

} // Path

}
FNGEOLIBUTIL_NAMESPACE_EXIT
