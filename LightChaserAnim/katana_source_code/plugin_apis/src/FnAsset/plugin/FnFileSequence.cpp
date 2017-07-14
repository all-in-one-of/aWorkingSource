// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAsset/plugin/FnFileSequence.h>
#include <FnAsset/plugin/FnAsset.h>
#include <FnLogging/FnLogging.h>

#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

//
// The Mapping for the internal Katana C-API on to the Plug-in C++ API Wrapper.
//

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

  void destroy(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      delete handle;
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("destroy", e);
    }
  }

  FnAttributeHandle getAsString(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string fileSeqStr;
      handle->getFileSequence().getAsString(fileSeqStr);
      return FnAttribute::StringAttribute(fileSeqStr.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getAsString", e);
    }
    return 0x0;
  }

  FnAttributeHandle getDirectory(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string directory;
      handle->getFileSequence().getDirectory(directory);
      return FnAttribute::StringAttribute(directory.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getDirectory", e);
    }
    return 0x0;
  }

  FnAttributeHandle getPrefix(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string prefix;
      handle->getFileSequence().getPrefix(prefix);
      return FnAttribute::StringAttribute(prefix.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getPrefix", e);
    }
    return 0x0;
  }

  FnAttributeHandle getSuffix(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string suffix;
      handle->getFileSequence().getSuffix(suffix);
      return FnAttribute::StringAttribute(suffix.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getSuffix", e);
    }
    return 0x0;
  }

  unsigned int getPadding(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getFileSequence().getPadding();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getPadding", e);
    }
    return 0;
 }

  FnAttributeHandle getResolvedPath(FnFileSequenceHandle handle, int frame, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::string path;
      handle->getFileSequence().getResolvedPath(frame, path);
      return FnAttribute::StringAttribute(path.c_str()).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getResolvedPath", e);
    }
    return 0x0;
  }

  bool hasFrameSet(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    return handle->getFileSequence().hasFrameSet();
  }

  bool isFrameInFrameSet(FnFileSequenceHandle handle, int frame, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getFileSequence().isFrameInFrameSet(frame);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("containsAssetId", e);
    }
    return false;
  }

  int getFirstFrameInFrameSet(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getFileSequence().getFirstFrameInFrameSet();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getFirstFrameInFrameSet", e);
    }
    return 0;
  }

  int getLastFrameInFrameSet(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getFileSequence().getLastFrameInFrameSet();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getLastFrameInFrameSet", e);
    }
    return 0;
  }

  void getNearestFramesInFrameSet(FnFileSequenceHandle handle, int frame,
    bool *hasLeft, int *nearestLeft, bool *hasRight, int *nearestRight,
    FnAttributeHandle *errorMessage)
  {
    try
    {
      handle->getFileSequence().getNearestFramesInFrameSet(
        frame, hasLeft, nearestLeft, hasRight, nearestRight);
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getNearestFramesInFrameSet", e);
    }
  }

  FnAttributeHandle getFrameListFromFrameSet(FnFileSequenceHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::vector<int> frameList;
      handle->getFileSequence().getFrameListFromFrameSet(frameList);
      return FnAttribute::IntAttribute(
            &frameList[0], frameList.size(), 1).getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getFrameListFromFrameSet", e);
    }
    return 0x0;
  }

  void destroyFileSequenceArray(FnFileSequenceArrayHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      delete handle;
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("destroyFileSequenceArray", e);
    }
  }

  unsigned int getFileSequenceCount(FnFileSequenceArrayHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      return handle->getFileSequenceArray().getFileSequenceCount();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getFileSequenceCount", e);
    }
    return 0;
  }

  FnFileSequenceHandle getFileSequence(FnFileSequenceArrayHandle handle,
                                       unsigned int index, FnAttributeHandle *errorMessage)
  {
    try
    {
      return Foundry::Katana::FileSequence::newFileSequenceHandle(handle->getFileSequenceArray().getFileSequence(index)->clone());
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getFileSequence", e);
    }
    return 0x0;
  }

  FnAttributeHandle getFileNames(FnFileSequenceArrayHandle handle, FnAttributeHandle *errorMessage)
  {
    try
    {
      std::vector<std::string> filenames;
      handle->getFileSequenceArray().getFileNames(filenames);
      FnAttribute::StringBuilder sb;
      sb.set(filenames);
      return sb.build().getRetainedHandle();
    }
    catch (std::exception &e)
    {
      *errorMessage = createError("getFileNames", e);
    }
    return 0x0;
  }
};  // anonymous namespace

namespace Foundry
{
  namespace Katana
  {
    FnFileSequenceHandle FileSequence::newFileSequenceHandle(FileSequence *fs)
    {
      if (!fs)
      {
        return 0x0;
      }

      if (!fs->isValid())
      {
        delete fs;
        return 0x0;
      }

      FnFileSequenceHandle h = new FnFileSequenceStruct(fs);
      return h;
    }

    FnFileSequenceArrayHandle FileSequence::newFileSequenceArrayHandle(FileSequenceArray* fsa)
    {
      if (!fsa)
      {
        return 0x0;
      }

      FnFileSequenceArrayHandle h = new FnFileSequenceArrayStruct(fsa);
      return h;
    }

    FnPlugStatus FileSequence::setHost(FnPluginHost *host)
    {
      _host = host;
      FnLogging::setHost(host);
      FnAttribute::GroupBuilder::setHost(host);
      return FnAttribute::Attribute::setHost(host);
    }

    FnPluginHost* FileSequence::getHost()
    {
      return _host;
    }

    FnFileSequencePluginSuite_v1 FileSequence::createSuite(
                    FnFileSequenceHandle (*create)(const char*, FnAttributeHandle *errorMessage),
                    bool (*isFileSequence)(const char*, FnAttributeHandle *errorMessage),
                    FnAttributeHandle (*buildFileSequenceString)(const char *prefix, const char *suffix, int padding, FnAttributeHandle *errorMessage),
                    FnFileSequenceArrayHandle (*findSequence)(const char** fileList, unsigned int fileCount, FnAttributeHandle *errorMessage))
    {
      FnFileSequencePluginSuite_v1 suite = {
                create,
                ::destroy,
                isFileSequence,
                buildFileSequenceString,
                ::getAsString,
                ::getDirectory,
                ::getPrefix,
                ::getSuffix,
                ::getPadding,
                ::getResolvedPath,
                ::hasFrameSet,
                ::isFrameInFrameSet,
                ::getFirstFrameInFrameSet,
                ::getLastFrameInFrameSet,
                ::getNearestFramesInFrameSet,
                ::getFrameListFromFrameSet,
                findSequence,
                ::destroyFileSequenceArray,
                ::getFileSequenceCount,
                ::getFileSequence,
                ::getFileNames
      };

      return suite;
    }

    unsigned int FileSequence::_apiVersion = 1;
    const char* FileSequence::_apiName = "FileSequencePlugin";
    FnPluginHost *FileSequence::_host = 0x0;

  }; // namespace Katana
};  // namespace Foundry
