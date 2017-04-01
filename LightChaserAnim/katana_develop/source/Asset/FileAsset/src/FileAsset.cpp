// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FileAsset.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>
#include <FnLogging/FnLogging.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#ifndef _WIN32
#include <pwd.h>
#endif
#include <boost/regex.hpp>

#include <pystring/pystring.h>


namespace
{
  FnKat::FnLogging::FnLog fileLog("File");

} // namespace anonymous

// This is the key for the 'type' attribute
// in an asset's attribute dictionary
//
const char * FileAsset::_typeAttributeKey("type");

FileAsset::FileAsset() : Asset()
{
  // Empty
}

FileAsset::~FileAsset() 
{
  // Empty
}

FnKat::Asset* FileAsset::create()
{
  return new FileAsset();
}

void FileAsset::reset()
{
  // Empty
}

bool FileAsset::isAssetId(const std::string& id)
{
    //very permissive... every string can be a file path... let's assume
    return true;
}

bool FileAsset::containsAssetId(const std::string& id)
{
    //very permissive... every string can be a file path... let's assume
    return true;
}

bool FileAsset::checkPermissions(const std::string &assetId, const StringMap &context)
{
  return true;
}

bool FileAsset::runAssetPluginCommand(const std::string &assetId, const std::string &command, const StringMap &commandArgs)
{
  return true;
}

void FileAsset::resolveAsset(const std::string& id, std::string& asset)
{
  asset = id;
}

void FileAsset::resolveAllAssets(const std::string& id, std::string& assets)
{
  assets = id;
}

void FileAsset::resolvePath(const std::string& str, const int frame, std::string& ret)
{
  ret = str;

  // Resolve user (~) and env vars ($)
  ret = _expandUser(ret);
  ret = _expandVars(ret);

  if (FnKat::DefaultFileSequencePlugin::isFileSequence(ret))
  {
      ret = FnKat::DefaultFileSequencePlugin::resolveFileSequence(ret, frame);
  }
}

void FileAsset::resolveAssetVersion(const std::string& assetId, std::string& ret, const std::string& versionStr)
{
  ret = "";
}

void FileAsset::getAssetDisplayName(const std::string& assetId, std::string& ret)
{
    std::string head;
    pystring::os::path::split(head, ret, assetId);
}

void FileAsset::getAssetVersions(const std::string& assetId, StringVector& ret)
{
    // No version support here
}

void FileAsset::getUniqueScenegraphLocationFromAssetId(const std::string& assetId, bool includeVersion, std::string& ret)
{
  std::vector<std::string> tokens;

  pystring::split(assetId.c_str(), tokens, "/");
  if (tokens.empty() || !tokens[0].empty())
  {
    tokens.insert(tokens.begin(), "");
  }

  for (unsigned int i=1; i<tokens.size(); ++i)
  {
    tokens[i] = _getSafeIdentifier(tokens[i]);
  }

  ret = pystring::join("/", tokens);
}

void FileAsset::getRelatedAssetId(const std::string& assetId, const std::string& relation, std::string& ret)
{
  ret = "";
}

void FileAsset::getAssetFields(const std::string& assetId, bool includeDefaults, StringMap& returnFields)
{
    std::string directory;
    std::string filename;
    std::string ext;

    // Get hold of the filename without extension
    pystring::os::path::split(directory, filename, assetId);
    pystring::os::path::splitext(filename, ext, filename);
    returnFields[kFnAssetFieldName] = filename;

    returnFields["path"] = std::string(assetId);
}

void FileAsset::buildAssetId(const StringMap& fields, std::string& ret)
{
    ret = "";

    StringMap::const_iterator iter, itend=fields.end();
    iter = fields.find("path");
    if (iter!=itend)
        ret = (*iter).second;
}

void FileAsset::getAssetAttributes(const std::string& assetId, const std::string& scope, StringMap& returnAttrs)
{
    // Make somewhere to store the
    // split path information
    //
    std::string root;
    std::string ext;
    const unsigned int REMOVE_DOT = 1;

    // Get hold of the file extension
    //
    pystring::os::path::splitext(root, ext, assetId);

    // Insert an error message in to the return
    // value if we were unable get a file extension.
    //
    if( ext.empty() )
    {
        ext = "Unknown: File path '";
        ext += assetId;
        ext += "'";
        ext += " has no extension";
    }

    // Chop off the dot at the start of the
    // file extension.
    //
    else
    {
        ext = ext.substr(REMOVE_DOT);
    }

    // Construct the key & value pair and insert it
    // in to the map. This will give us a map that
    // contains a 'type' key with it's value
    // as the assets file extension without the dot.
    //
    std::pair<std::string, std::string> entry(FileAsset::_typeAttributeKey, ext);
    returnAttrs.insert( entry );
}

void FileAsset::setAssetAttributes(const std::string& assetId, const std::string& scope, const StringMap& attrs)
{
  // Empty
}

void FileAsset::getAssetIdForScope(const std::string& assetId, const std::string& scope, std::string& ret)
{
  ret = assetId;
}

void FileAsset::createAssetAndPath(FnKat::AssetTransaction* txn, const std::string& assetType,
    const StringMap& assetFields, const StringMap& args, bool createDirectory, std::string& assetId)
{
    buildAssetId(assetFields, assetId);

    if (assetType == kFnAssetTypeKatanaScene) {
        if(!pystring::endswith(assetId, ".katana")) {
            assetId += ".katana";
        }
    } else if (assetType == kFnAssetTypeLookFile) {
        if(!pystring::endswith(assetId, ".klf")) {
            assetId += ".klf";
        }
    } else if (assetType == kFnAssetTypeFCurveFile) {
        if(!pystring::endswith(assetId, ".fcurve")) {
            assetId += ".fcurve";
        }
    } else if (assetType == kFnAssetTypeLiveGroup) {
        if(!pystring::endswith(assetId, ".livegroup")) {
            assetId += ".livegroup";
        }
    }
}

void  FileAsset::postCreateAsset(FnKat::AssetTransaction* txn, const std::string& assetType,
              const StringMap& assetFields, const StringMap& args,
              std::string& assetId)
{
    buildAssetId(assetFields, assetId);
}


// --- Private helper methods  ------------------------

std::string FileAsset::_getSafeIdentifier(const std::string& id)
{
  std::string result;
  std::string::const_iterator it = id.begin(), itEnd = id.end();
  for (; it != itEnd; ++it)
  {
    if (isalnum(*it) || *it == '_')
    {
      result += *it;
    }
    else
    {
      result += '_';
    }
  }
  
  if (result.empty())
  {
    result = "_";
  }

  if (!isalpha(result[0]) && result[0] != '_')
  {
    result = std::string("_") + result;
  }

  return result;
}

std::string FileAsset::_expandUser(const std::string& fileString)
{
  if (!pystring::startswith(fileString, "~"))
    return fileString;

  std::string userHome;

  int i = pystring::find(fileString, "/", 1);
  if (i < 0)
  {
    i = fileString.size();
  }
  if (i == 1)
  {
    char* homeenv = getenv("HOME");
    if (homeenv)
    {
      userHome = homeenv;
    }
    else
    {
#ifndef _WIN32
      struct passwd* pwd = getpwuid(getuid());
      if (pwd)
      {
        userHome = pwd->pw_dir;
      }
#endif
    }
  }
  else
  {
    std::string userName = pystring::slice(fileString, 1, i);
#ifndef _WIN32
    struct passwd* pwd = getpwnam(userName.c_str());
    if (pwd)
    {
      userHome = pwd->pw_dir;
    }
#endif
  }

  if (userHome.empty())
  {
    return fileString;
  }

  if (pystring::endswith(userHome, "/"))
  {
    i += 1;
  }

  return userHome + pystring::slice(fileString, i);
}

static boost::regex g_varExp("\\$(\\w+|\\{[^}]*\\})",
                             boost::regex::extended & ~boost::regex::collate);

std::string FileAsset::_expandVars(const std::string& fileString)
{
  if (pystring::find(fileString, "$") < 0)
  {
    return fileString;
  }

  std::string returnString = fileString;

  int i=0;
  int offset = 0;
  while (true)
  {
    boost::smatch m;
    std::string substring = returnString.substr(offset);
    if (!boost::regex_search(substring, m, g_varExp))
    {
      break;
    }

    // replace the match in returnString
    i = m.position() + offset;
    int j = m.position() + m.length() + offset;

    std::string name = pystring::slice(returnString, i + 1, j);

    if (pystring::startswith(name, "{") && pystring::endswith(name, "}"))
    {
      name = pystring::slice(name, 1, -1);
    }
    const char* nameEnv = getenv(name.c_str());
    if (nameEnv)
    {
      std::string tail = pystring::slice(returnString, j);
      returnString = pystring::slice(returnString, 0, i) + nameEnv;
      i = returnString.size();
      returnString += tail;
    }
    else
    {
      i = j;
      offset++;
    }
  };

  return returnString;
}


// --- Register plugin ------------------------

DEFINE_ASSET_PLUGIN(FileAsset)

void registerPlugins()
{
  REGISTER_PLUGIN(FileAsset, "File", 0, 1);
}
