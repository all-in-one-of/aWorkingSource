// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNFILESEQ__H
#define FNFILESEQ__H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAsset/suite/FnAssetSuite.h>

#include <FnAttribute/FnAttribute.h>

#include <vector>
#include <string>
#include <memory>

namespace Foundry
{
  namespace Katana
  {
    class FileSequenceArray;

    /**
     * \defgroup FnAsset Asset Plug-In API
     * @{
     *
     *
     */

   /**
     * @brief Primary class to derive from when implementing file sequence plugin.
     *
     * File sequences are closely related to the asset system.  A file sequence spec is a
     * file path that contains tokens to be substituted with the current frame number.  Some
     * file sequence specs may also specify the frame range of the sequence.
     *
     * An example from the FileSeq plugin: <code>/path/to/image.1-10#.exr</code>
     *
     * This example references a sequence of images, frames 1 through 10, with the frame
     * number zero-padded to length 4.
     *
     * Resolving this sequence at frame 5 would result in: <code>/path/to/image.0005.exr</code>
     *
     * In addition to implementing all the pure virtual functions in this class, you must also implement static
     * methods in your derived class:
     *
     * \code
     * // return a new instance of your derived class, owned by caller.
     * static FileSequence * create(const char *);
     *
     * // return true if string is a valid file sequence
     * static bool isFileSequence(const char*);
     *
     * // return a valid file sequence string built from input prefix/suffix/padding
     * std::string buildFileSequenceString(const std::string &prefix, const std::string &suffix, int padding);
     *
     * // return a new FileSequenceArray instance containing sequences and non-sequence file paths found
     * // in input file path list
     * FileSequenceArray* findSequence(const char** fileList, unsigned int fileCount);
     * \endcode
     *
     * To indicate an error to the caller, your implemntation may throw an
     * exception derived from std::exception.
     */
    class FileSequence
    {
    public:
      /** @brief Construct a file sequence object instance for a given file sequence string.
       * @param fileseq String representing file sequence (e.g. "image.#.exr" or "image.%04d.exr").
       */
      FileSequence(const std::string& fileseq) { /* Empty */ }
      virtual ~FileSequence() { /* Empty */ }

      /** @brief Return whether this instance contains a valid file sequence.
       */
      virtual bool          isValid() const = 0;

      /** @brief Return new copy of this class instance. */
      virtual FileSequence* clone() const = 0;

      /** @brief Get string representation of the file sequence
       *
       * @param retValue Set to file sequence string.
       */
      virtual void          getAsString(std::string& retValue) = 0;

      /** @brief Get the directory portion of file sequence
       *
       * @param retValue Set to directory portion of file sequence (e.g. "/a/b/c" for fileseq "/a/b/c/d.#.exr").
       */
      virtual void          getDirectory(std::string& retValue) = 0;

      /** @brief Get the portion of file sequence before the frame number.
       *
       * @param retValue Set to prefix portion of file sequence (e.g. "/a/b/c/d." for fileseq "/a/b/c/d.#.exr").
       */
      virtual void          getPrefix(std::string& retValue) = 0;

      /** @brief Get the portion of file sequence after the frame number.
       *
       * @param retValue Set to suffix portion of file sequence (e.g. ".exr" for fileseq "/a/b/c/d.#.exr").
       */
      virtual void          getSuffix(std::string& retValue) = 0;

      /** @brief Get the padding of the file sequence frame number.
       *
       * @return Padding amount of file sequence (e.g. 4 for fileseq that expands to image.0001.exr)
       */
      virtual unsigned int  getPadding() = 0;

      /** @brief Get the filename for file sequence with sequence portion replaced by correctly padded frame.
       * @param frame The frame to get the filename.
       *
       * @param retValue Set to resolved filename.
       */
      virtual void          getResolvedPath(const int frame, std::string& retValue) = 0;

      /** @brief Return whether file sequence has explicit frame set.
       *
       * @return True if file sequence has explicit frame range (set) embedded in it (e.g. "image.1-10#.exr").
       */
      virtual bool          hasFrameSet() = 0;

      /** @brief Test whether frame is within explicit frame set.
       *
       * @return True if frame is in frame set.  If file sequence does not have explicit frame set, always return true.
       */
      virtual bool          isFrameInFrameSet(const int frame) = 0;

      /** @brief Return first frame in explicit frame set.
       *
       * @return First (minimum) frame in frame set.  If file sequence does not have explicit frame set, always return 0.
       */
      virtual int           getFirstFrameInFrameSet() = 0;

      /** @brief Return last frame in explicit frame set.
       *
       * @return Last (maximum) frame in frame set.  If file sequence does not have explicit frame set, always return 0.
       */
      virtual int           getLastFrameInFrameSet() = 0;

      /** @brief Return nearest frame to left and right of frame
       *
       * @param frame Frame number to return nearest frames of.
       * @param hasLeft Set to true if frame exists in list to left of specified frame.
       * @param nearestLeft Set to nearest frame in list to left of specified frame.
       * @param hasRight Set to true if frame exists in list to right of specified frame.
       * @param nearestRight Set to nearest frame in list to right of specified frame.
       *
       * Set return values to (false, 0, false, 0) if file sequence does not have explicit fram set.
       */
      virtual void          getNearestFramesInFrameSet(int frame,
                                bool *hasLeft, int *nearestLeft,
                                bool *hasRight, int *nearestRight) = 0;

      /** @brief Get frame set as list of integers.
       *
       * @param returnList Fill with integer frame numbers (e.g. [1,2,3,4,5] for "image.1-5#.exr").
       */
      virtual void          getFrameListFromFrameSet(std::vector<int>& returnList) = 0;

      ///@cond FN_INTERNAL_DEV
      static FnPlugStatus setHost(FnPluginHost *host);
      static FnPluginHost *getHost();

      static FnFileSequencePluginSuite_v1 createSuite(
        FnFileSequenceHandle (*create)(const char*, FnAttributeHandle *errorMessage),
        bool (*isFileSequence)(const char*, FnAttributeHandle *errorMessage),
        FnAttributeHandle (*buildFileSequenceString)(const char *prefix, const char *suffix, int padding, FnAttributeHandle *errorMessage),
        FnFileSequenceArrayHandle (*findSequence)(const char** fileList, unsigned int fileCount, FnAttributeHandle *errorMessage));

      static FnFileSequenceHandle newFileSequenceHandle(FileSequence *fs);
      static FnFileSequenceArrayHandle newFileSequenceArrayHandle(FileSequenceArray* fs);

      static unsigned int _apiVersion;
      static const char*  _apiName;

    private:
      static FnPluginHost *_host;

      ///@endcond
    };

    /**
      * @brief Container class that holds a vector of FileSequence instances and a vector of non-sequence
      *  filenames.
      *
      */
    class FileSequenceArray
    {
    public:
      FileSequenceArray(std::vector<FileSequence*>& seq, std::vector<std::string>& nonseq) :
        _seq(seq),
        _nonseq(nonseq)
      {
        // Empty
      }

      virtual ~FileSequenceArray()
      {
        for (unsigned int i=0; i<_seq.size(); ++i)
        {
          delete _seq[i];
          _seq[i] = 0;
        }
        _seq.clear();
      }

      /**
        * @brief Number of file sequence objects in vector.
        */
      virtual int getFileSequenceCount()
      {
        return _seq.size();
      }

      /**
        * @brief Get FileSequence object ptr by index.
        * The caller does not own the FileSequence object returned.
        */
      virtual FileSequence* getFileSequence(const unsigned int index)
      {
        return _seq[index];
      }

      /**
        * @brief Get vector of non-sequence file names.
        */
      virtual void getFileNames(std::vector<std::string>& fileNames)
      {
        fileNames = _nonseq;
      }

    private:
      std::vector<FileSequence*>        _seq;
      std::vector<std::string>          _nonseq;
    };

    /** @} */

  };  // namespace Katana
}; // namespace Foundry

namespace FnKat = Foundry::Katana;

///@cond FN_FOUNDRY_DEV

// Plugin Registering Macro.

#define DEFINE_FS_PLUGIN(FS_CLASS)                                                              \
                                                                                                \
  FnPlugin FS_CLASS##_plugin;                                                                   \
                                                                                                \
  FnFileSequenceHandle FS_CLASS##_create(const char* fileseq, FnAttributeHandle *errorMessage)  \
  {                                                                                             \
    try                                                                                         \
    {                                                                                           \
      return Foundry::Katana::FileSequence::newFileSequenceHandle(FS_CLASS::create(fileseq));   \
    }                                                                                           \
    catch (std::exception & e)                                                                  \
    {                                                                                           \
      Foundry::Katana::Attribute errorAttr = Foundry::Katana::StringAttribute(e.what());        \
      *errorMessage = errorAttr.getRetainedHandle();                                            \
      return 0x0;                                                                               \
    }                                                                                           \
  }                                                                                             \
                                                                                                \
  bool FS_CLASS##_isFileSequence(const char* fileseq, FnAttributeHandle *errorMessage)          \
  {                                                                                             \
    try                                                                                         \
    {                                                                                           \
      return FS_CLASS::isFileSequence(fileseq);                                                 \
    }                                                                                           \
    catch (std::exception & e)                                                                  \
    {                                                                                           \
      Foundry::Katana::Attribute errorAttr = Foundry::Katana::StringAttribute(e.what());        \
      *errorMessage = errorAttr.getRetainedHandle();                                            \
      return false;                                                                             \
    }                                                                                           \
  }                                                                                             \
                                                                                                \
                                                                                                \
  FnAttributeHandle FS_CLASS##_buildFileSequenceString(const char *prefix,                      \
                                                       const char *suffix,                      \
                                                       int padding,                             \
                                                       FnAttributeHandle *errorMessage)         \
  {                                                                                             \
    try                                                                                         \
    {                                                                                           \
      std::string prefixStr = std::string(prefix);                                              \
      std::string suffixStr  = std::string(suffix);                                             \
      std::string fileSeqStr = FS_CLASS::buildFileSequenceString(prefixStr, suffixStr, padding);\
      FnAttribute::Attribute resultAttr = FnAttribute::StringAttribute(fileSeqStr.c_str());     \
      return resultAttr.getRetainedHandle();                                                    \
    }                                                                                           \
    catch (std::exception & e)                                                                  \
    {                                                                                           \
      Foundry::Katana::Attribute errorAttr = Foundry::Katana::StringAttribute(e.what());        \
      *errorMessage = errorAttr.getRetainedHandle();                                            \
      return 0x0;                                                                               \
    }                                                                                           \
  }                                                                                             \
                                                                                                \
                                                                                                \
  FnFileSequenceArrayHandle FS_CLASS##_findSequence(const char** fileList,                      \
                                                    unsigned int fileCount,                     \
                                                    FnAttributeHandle *errorMessage)            \
  {                                                                                             \
    try                                                                                         \
    {                                                                                           \
      return Foundry::Katana::FileSequence::newFileSequenceArrayHandle(                         \
        FS_CLASS::findSequence(fileList, fileCount));                                           \
    }                                                                                           \
    catch (std::exception & e)                                                                  \
    {                                                                                           \
      Foundry::Katana::Attribute errorAttr = Foundry::Katana::StringAttribute(e.what());        \
      *errorMessage = errorAttr.getRetainedHandle();                                            \
      return 0x0;                                                                               \
    }                                                                                           \
  }                                                                                             \
                                                                                                \
  FnFileSequencePluginSuite_v1 FS_CLASS##_suite =                                               \
    Foundry::Katana::FileSequence::createSuite(FS_CLASS##_create,                               \
                                               FS_CLASS##_isFileSequence,                       \
                                               FS_CLASS##_buildFileSequenceString,              \
                                               FS_CLASS##_findSequence);                        \
                                                                                                \
  const void* FS_CLASS##_getSuite()                                                             \
  {                                                                                             \
    return &FS_CLASS##_suite;                                                                   \
  }                                                                                             \



class FnFileSequenceStruct
{
public:
  FnFileSequenceStruct(Foundry::Katana::FileSequence *context) :
    _fileSequence(context)
  {
    // Empty
  }

  ~FnFileSequenceStruct() { /* Empty */ };

  Foundry::Katana::FileSequence& getFileSequence()
  {
    return *_fileSequence;
  }

private:
  std::auto_ptr<Foundry::Katana::FileSequence> _fileSequence;
};

class FnFileSequenceArrayStruct
{
  public:
    FnFileSequenceArrayStruct(Foundry::Katana::FileSequenceArray* context) :
      _fileSequenceArray(context)
    {
      // Empty
    }

    ~FnFileSequenceArrayStruct()
    {
      // Empty
    }

    Foundry::Katana::FileSequenceArray& getFileSequenceArray()
    {
      return *_fileSequenceArray;
    }

  private:
    std::auto_ptr<Foundry::Katana::FileSequenceArray> _fileSequenceArray;
};


///@endcond

#endif  // #ifndef FNFILESEQ__H
