// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAsset/plugin/FnAsset.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>
#include <FnLogging/FnLogging.h>

#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <iostream>

namespace
{
  FnAttributeHandle createError(std::string functionName, std::exception &e)
  {
    std::string errorMsg("");;
    errorMsg += "Host '";
    errorMsg += functionName;
    errorMsg += "' error: ";
    errorMsg += e.what();

    return FnAttribute::StringAttribute(errorMsg).getRetainedHandle();
  }

  void destroy(FnAssetHandle handle)
  {
    delete handle;
  }

  void reset(FnAssetHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      handle->getAsset().reset();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("reset", e);
    }
  }

  bool isAssetId(FnAssetHandle handle, const char* str, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getAsset().isAssetId(str);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("isAssetId", e);
    }
    return false;
  }

  bool containsAssetId(FnAssetHandle handle, const char* str, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getAsset().containsAssetId(str);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("containsAssetId", e);
    }
    return false;
  }

  bool checkPermissions(FnAssetHandle handle, const char *assetId, FnAttributeHandle context,
                        FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap contextMap;

      FnAttribute::GroupAttribute contextGroup = FnAttribute::Attribute::CreateAndRetain(context);
      if (contextGroup.isValid())
      {
          const int count = contextGroup.getNumberOfChildren();
          for (int i = 0; i < count; ++i)
          {
              const std::string childName = contextGroup.getChildName(i);
              if (!childName.empty())
              {
                  contextMap.insert(std::pair<std::string, std::string>(
                    childName, FnAttribute::StringAttribute(contextGroup.getChildByIndex(i)).getValue("", false)));
              }
          }
      }

      return handle->getAsset().checkPermissions(assetId, contextMap);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("checkPermissions", e);
    }
    return false;
  }

  bool runAssetPluginCommand(FnAssetHandle handle, const char *assetId, const char *command,
                             FnAttributeHandle commandArgs, FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap args;

      FnAttribute::GroupAttribute argsGroup = FnAttribute::Attribute::CreateAndRetain(commandArgs);
      if (argsGroup.isValid())
      {
          const int count = argsGroup.getNumberOfChildren();
          for (int i = 0; i < count; ++i)
          {
              const std::string childName = argsGroup.getChildName(i);
              if (!childName.empty())
              {
                  args.insert(std::pair<std::string, std::string>(
                    childName, FnAttribute::StringAttribute(argsGroup.getChildByIndex(i)).getValue("", false)));
              }
          }
      }

      return handle->getAsset().runAssetPluginCommand(assetId, command, args);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("runAssetPluginCommand", e);
    }
    return false;
  }

  FnAttributeHandle resolveAsset(FnAssetHandle handle, const char* assetId, FnAttributeHandle *errorMessage)
  {
    std::string ret = assetId;
    try
    {
      handle->getAsset().resolveAsset(assetId, ret);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("resolveAsset", e);
    }
    return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
  }

  FnAttributeHandle resolveAllAssets(FnAssetHandle handle, const char* str, FnAttributeHandle *errorMessage)
  {
    std::string ret = str;
    try
    {
      handle->getAsset().resolveAllAssets(str, ret);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("resolveAllAssets", e);
    }
    return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
  }

  FnAttributeHandle resolvePath(FnAssetHandle handle, const char* path, int frame, FnAttributeHandle *errorMessage)
  {
    std::string ret = path;
    try
    {
      handle->getAsset().resolvePath(path, frame, ret);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("resolvePath", e);
    }
    return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
  }

  FnAttributeHandle resolveAssetVersion(FnAssetHandle handle, const char* assetId, const char* versionStr, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string ret;
      handle->getAsset().resolveAssetVersion(assetId, ret, versionStr);
      return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("resolveAssetVersion", e);
    }
    return 0x0;
  }

  FnAttributeHandle getAssetDisplayName(FnAssetHandle handle, const char* assetId, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string ret;
      handle->getAsset().getAssetDisplayName(assetId, ret);
      return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getAssetDisplayName", e);
    }
    return 0x0;
  }

  FnAttributeHandle getAssetVersions(FnAssetHandle handle, const char* assetId, FnAttributeHandle *errorMessage)
  {
    try
    {
      // Get hold of the versions.
      //
      typedef std::vector<std::string> StringVector;
      StringVector versions;
      handle->getAsset().getAssetVersions(assetId, versions);

      // If there is anything to get hold of
      // then copy it across to katana.
      //
      if(!versions.empty())
      {
        const size_t versionCount = versions.size();

        // Convert std::vector<std::string>
        // to
        // std::vector<char*>
        //
        // So that we can pass char** to StringAttribute.
        //
        // Using StringAttribute allows us to avoid memory
        // ownership problems.
        //
        const size_t FRONT = 0;
        std::vector<const char*> versionsArray(versionCount);
        for(size_t i=0; i != versionCount; ++i)
        {
          versionsArray[i] = &(versions[i][FRONT]);
        }

        // Copy all the data in to a StringAttribute
        //
        const int ONE_DIMENSIONAL = 1;
        FnAttribute::StringAttribute attrVersions(&(versionsArray[FRONT]), versionCount, ONE_DIMENSIONAL);

        return attrVersions.getRetainedHandle();
      }

      // Nothing to return
      //
      return 0x0;
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getAssetVersions", e);
    }
    return 0x0;
  }

  FnAttributeHandle getUniqueScenegraphLocationFromAssetId(FnAssetHandle handle, const char* assetId,
                                                           bool includeVersion, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string ret;
      handle->getAsset().getUniqueScenegraphLocationFromAssetId(assetId, includeVersion, ret);
      return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getUniqueScenegraphLocationFromAssetId", e);
    }
    return 0x0;
  }

  FnAttributeHandle getRelatedAssetId(FnAssetHandle handle, const char* assetId, const char* relation, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string ret;
      handle->getAsset().getRelatedAssetId(assetId, relation, ret);
      return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getRelatedAssetId", e);
    }
    return 0x0;
  }

  FnAttributeHandle getAssetFields(FnAssetHandle handle, const char* assetId, bool includeDefaults, FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap assetFields;
      handle->getAsset().getAssetFields(assetId, includeDefaults, assetFields);

      FnAttribute::GroupBuilder gb;
      Foundry::Katana::Asset::StringMap::const_iterator it = assetFields.begin(), itEnd = assetFields.end();
      for (; it != itEnd; ++it)
      {
        gb.set(it->first, FnAttribute::StringAttribute(it->second.c_str()));
      }
      return gb.build().getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getAssetFields", e);
    }
    return 0x0;
  }

  FnAttributeHandle buildAssetId(FnAssetHandle handle, FnAttributeHandle fields, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string ret;
      Foundry::Katana::Asset::StringMap inFields;

      FnAttribute::GroupAttribute grpFields = FnAttribute::Attribute::CreateAndRetain(fields);
      if (grpFields.isValid())
      {
        unsigned int count = grpFields.getNumberOfChildren();
        for (unsigned int i=0; i<count; ++i)
        {
          std::string childName = grpFields.getChildName(i);
          if (!childName.empty())
          {
            inFields.insert(std::pair<std::string, std::string>(childName, FnAttribute::StringAttribute(grpFields.getChildByIndex(i)).getValue("", false)));
          }
        }
      }

      handle->getAsset().buildAssetId(inFields, ret);
      return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("buildAssetId", e);
    }
    return 0x0;
  }

  FnAttributeHandle getAssetAttributes(FnAssetHandle handle, const char* assetId, const char* scope, FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap returnAttrs;
      handle->getAsset().getAssetAttributes(assetId, scope, returnAttrs);

      FnAttribute::GroupBuilder gb;
      Foundry::Katana::Asset::StringMap::const_iterator it = returnAttrs.begin(), itEnd = returnAttrs.end();
      for (; it != itEnd; ++it)
      {
        gb.set(it->first, FnAttribute::StringAttribute(it->second.c_str()));
      }
      return gb.build().getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getAssetAttributes", e);
    }
    return 0x0;
  }

  void setAssetAttributes(FnAssetHandle handle, const char* assetId, const char* scope, FnAttributeHandle attributes, FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap setAttrs;

      FnAttribute::GroupAttribute attributesAttr = FnAttribute::Attribute::CreateAndRetain(attributes);
      if (attributesAttr.isValid())
      {
        int numChildren = attributesAttr.getNumberOfChildren();
        for (int i=0; i<numChildren; ++i)
        {
          FnAttribute::StringAttribute childAttr = attributesAttr.getChildByIndex(i);
          if (childAttr.isValid())
          {
            setAttrs[attributesAttr.getChildName(i)] = childAttr.getValue("", false);
          }
        }
        handle->getAsset().setAssetAttributes(assetId, scope, setAttrs);
      }
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("setAssetAttributes", e);
    }
  }

  FnAttributeHandle getAssetIdForScope(FnAssetHandle handle, const char* assetId, const char* scope, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string ret;
      handle->getAsset().getAssetIdForScope(assetId, scope, ret);
      return FnAttribute::StringAttribute(ret.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getAssetIdForScope", e);
    }
    return 0x0;
  }

  FnAssetTransactionHandle createTransaction(FnAssetHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::AssetTransaction* txn = handle->getAsset().createTransaction();
      return Foundry::Katana::Asset::newAssetTransactionHandle(txn);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("createTransaction", e);
    }
    return 0x0;
  }

  void cancelTransaction(FnAssetTransactionHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      handle->getAssetTransaction().cancel();
      delete handle;
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("cancelTransaction", e);
    }
  }

  bool commitTransaction(FnAssetTransactionHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      bool result = handle->getAssetTransaction().commit();
      delete handle;
      return result;
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("commitTransaction", e);
    }
    return false;
  }

  FnAttributeHandle createAssetAndPath(FnAssetHandle handle, FnAssetTransactionHandle txn, const char* assetType,
                                       FnAttributeHandle assetFields, FnAttributeHandle args, bool createDirectory,
                                       FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap inAssetFields;
      Foundry::Katana::Asset::StringMap inArgs;
      std::string assetId;

      FnAttribute::GroupAttribute grpAssetFields = FnAttribute::Attribute::CreateAndRetain(assetFields);
      if (grpAssetFields.isValid())
      {
        unsigned int count = grpAssetFields.getNumberOfChildren();
        for (unsigned int i=0; i<count; ++i)
        {
          std::string childName = FnAttribute::DelimiterDecode(
            grpAssetFields.getChildName(i));
          if (!childName.empty())
          {
            inAssetFields.insert(
              std::pair<std::string, std::string>(
                childName,
                FnAttribute::StringAttribute(grpAssetFields.getChildByIndex(i)).getValue("", false)));
          }
        }
      }

      FnAttribute::GroupAttribute grpArgs = FnAttribute::Attribute::CreateAndRetain(args);
      if (grpArgs.isValid())
      {
        unsigned int count = grpArgs.getNumberOfChildren();
        for (unsigned int i=0; i<count; ++i)
        {
          std::string childName = FnAttribute::DelimiterDecode(
            grpArgs.getChildName(i));
          if (!childName.empty())
          {
            inArgs.insert(
              std::pair<std::string, std::string>(
                childName,
                FnAttribute::StringAttribute(grpArgs.getChildByIndex(i)).getValue("", false)));
          }
        }
      }

      handle->getAsset().createAssetAndPath((txn)?(&txn->getAssetTransaction()):0x0, assetType, inAssetFields,
        inArgs, createDirectory, assetId);

      return FnAttribute::StringAttribute(assetId.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("createAssetAndPath", e);
    }
    return 0x0;
  }


  FnAttributeHandle postCreateAsset(FnAssetHandle handle, FnAssetTransactionHandle txn, const char* assetType,
                                         FnAttributeHandle assetFields, FnAttributeHandle args,
                                         FnAttributeHandle *errorMessage)
  {
    try
    {
      Foundry::Katana::Asset::StringMap inAssetFields;
      Foundry::Katana::Asset::StringMap inArgs;
      std::string assetId;

      FnAttribute::GroupAttribute grpAssetFields = FnAttribute::Attribute::CreateAndRetain(assetFields);
      if (grpAssetFields.isValid())
      {
        unsigned int count = grpAssetFields.getNumberOfChildren();
        for (unsigned int i=0; i<count; ++i)
        {
          std::string childName = grpAssetFields.getChildName(i);
          if (!childName.empty())
          {
            inAssetFields.insert(
              std::pair<std::string, std::string>(
                childName,
                FnAttribute::StringAttribute(grpAssetFields.getChildByIndex(i)).getValue("", false)));
          }
        }
      }

      FnAttribute::GroupAttribute grpArgs = FnAttribute::Attribute::CreateAndRetain(args);
      if (grpArgs.isValid())
      {
        unsigned int count = grpArgs.getNumberOfChildren();
        for (unsigned int i=0; i<count; ++i)
        {
          std::string childName = grpArgs.getChildName(i);
          if (!childName.empty())
          {
            inArgs.insert(
              std::pair<std::string, std::string>(
                childName,
                FnAttribute::StringAttribute(grpArgs.getChildByIndex(i)).getValue("", false)));
          }
        }
      }

      handle->getAsset().postCreateAsset((txn)?(&txn->getAssetTransaction()):0x0, assetType, inAssetFields,
        inArgs, assetId);

      return FnAttribute::StringAttribute(assetId.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("postCreateAsset", e);
    }
    return 0x0;
  }



}

namespace Foundry
{
  namespace Katana
  {
    AssetTransaction * Asset::createTransaction()
    {
      return new SimpleTransaction();
    }

    FnAssetHandle Asset::newAssetHandle(Asset* asset)
    {
      if (!asset)
      {
        return 0x0;
      }

      FnAssetHandle h = new FnAssetStruct(asset);
      return h;
    }

    FnAssetTransactionHandle Asset::newAssetTransactionHandle(AssetTransaction* txn)
    {
      if (!txn)
      {
        return 0x0;
      }

      FnAssetTransactionHandle h = new FnAssetTransactionStruct(txn);
      return h;
    }

    FnPlugStatus Asset::setHost(FnPluginHost* host)
    {
      _host = host;
      FnLogging::setHost(host);
      Foundry::Katana::DefaultFileSequencePlugin::setHost(host);
      FnAttribute::GroupBuilder::setHost(host);
      return FnAttribute::Attribute::setHost(host);
    }

    FnPluginHost* Asset::getHost()
    {
      return _host;
    }

    FnAssetPluginSuite_v1 Asset::createSuite(FnAssetHandle (*create)())
    {
      FnAssetPluginSuite_v1 suite =
      {
        create,
        ::destroy,
        ::reset,
        ::isAssetId,
        ::containsAssetId,
        ::checkPermissions,
        ::runAssetPluginCommand,
        ::resolveAsset,
        ::resolveAllAssets,
        ::resolvePath,
        ::resolveAssetVersion,
        ::getAssetDisplayName,
        ::getAssetVersions,
        ::getUniqueScenegraphLocationFromAssetId,
        ::getRelatedAssetId,
        ::getAssetFields,
        ::buildAssetId,
        ::getAssetAttributes,
        ::setAssetAttributes,
        ::getAssetIdForScope,
        ::createTransaction,
        ::cancelTransaction,
        ::commitTransaction,
        ::createAssetAndPath,
        ::postCreateAsset
      };

      return suite;
    };

    unsigned int Asset::_apiVersion = 1;
    const char* Asset::_apiName = "AssetPlugin";
    FnPluginHost* Asset::_host = 0x0;

    //SimpleAction

    SimpleTransaction::~SimpleTransaction()
    {
        _clearActions();
    }

    void SimpleTransaction::addAction(SimpleAction *action)
    {
        if (action)
        {
          _actions.push_back(action);
        }
    }

    void SimpleTransaction::cancel()
    {
        _clearActions();
    }

    bool SimpleTransaction::commit()
    {
        for (unsigned int i=0; i<_actions.size(); ++i)
        {
            if (!_actions[i]->doit())
            {
                for (int j=i-1; j>=0; --j)
                {
                    _actions[j]->rollback();
                }
                return false;
            }
        }

        _clearActions();
        return true;
    }

    void SimpleTransaction::_clearActions()
    {
        for (std::vector<SimpleAction *>::iterator iter=_actions.begin();
             iter!=_actions.end(); ++iter)
        {
          delete (*iter);
        }
        _actions.clear();
    }
  } // namespace Katana
} // namespace Foundry


