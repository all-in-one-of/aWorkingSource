// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <errno.h>
#include <netdb.h>
#include <string>
#include <math.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <map>

#include <ndspy.h> // prman display driver header.

#include <FnDisplayDriver/FnKatanaDisplayDriver.h>
namespace FnKat = Foundry::Katana;

//#define DRIVER_DEBUG 1
#ifdef DRIVER_DEBUG
  #include <iostream>
#endif

/* This is the user structure that prman will hang onto
   per instance of this display driver.
*/
struct UserImageStruct
{
    int scale;
    FnKat::KatanaPipe *pipe;
    FnKat::NewChannelMessage *channel;
    UserImageStruct() : scale(1), pipe(0), channel(0) {}
};

unsigned char *gScaledata = NULL;
FnKat::NewChannelMessage* gChannelMsg = NULL;

/*******************************************************************************
 *                      RENDERMAN DISPLAY DRIVER HELPER
 * This is a helper class that helps us manage Frame & Channels and
 * a couple of utility functions.
 ******************************************************************************/
class PRManKatanaDisplayDriverHelper
{
public:
  static PtDspyError parseHostName(const char* hostnameString, char* hostName, unsigned long* portNumber);

  static PRManKatanaDisplayDriverHelper* Instance();
  static void Destroy();

  // Do we already have a NewFrameMessage object for the frameID?
  bool haveObjectForFrame(int frameID);
  // Add an NewFrameMessage object for the given frame ID
  void addNewFrameObject(int frameID, FnKat::NewFrameMessage* frame);
  // Get the NewFrameMessage object for a given frameID
  FnKat::NewFrameMessage* getFrameObject(int frameID);

  // Get a channelID for the frame...
  int getNextChannelID(FnKat::NewFrameMessage* frame);

  void addChannelObject(const std::string &name, FnKat::NewChannelMessage* channel);

  FnKat::NewChannelMessage* getChannelObjectFor(const std::string &name);

private:
  PRManKatanaDisplayDriverHelper() : _frameMap(), _channelMap(), _channelObjectMap() {};
  PRManKatanaDisplayDriverHelper(const PRManKatanaDisplayDriverHelper& rhs);
  ~PRManKatanaDisplayDriverHelper() {};

  static PRManKatanaDisplayDriverHelper* _instance;

  std::map<int, FnKat::NewFrameMessage* > _frameMap;
  std::map<FnKat::NewFrameMessage*, int > _channelMap;

  // This map allows lower res versions of the same channel to reference the
  // same channel object.
  std::map<std::string, FnKat::NewChannelMessage* > _channelObjectMap;
};

PRManKatanaDisplayDriverHelper* PRManKatanaDisplayDriverHelper::_instance = NULL;

PRManKatanaDisplayDriverHelper* PRManKatanaDisplayDriverHelper::Instance()
{
  if(!_instance)
    _instance = new PRManKatanaDisplayDriverHelper();

  return _instance;
}

void PRManKatanaDisplayDriverHelper::Destroy()
{
  if(!_instance)
  {
    delete _instance;
    _instance = NULL;
  }
}

void PRManKatanaDisplayDriverHelper::addChannelObject(const std::string &name, FnKat::NewChannelMessage* channel)
{
  _channelObjectMap[name] = channel;
}

FnKat::NewChannelMessage* PRManKatanaDisplayDriverHelper::getChannelObjectFor(const std::string &name)
{
  return _channelObjectMap[name];
}

PtDspyError PRManKatanaDisplayDriverHelper::parseHostName(const char* hostnameString, char* hostName, unsigned long* portNumber)
{
  // Skip any leading _'s
  while (*hostnameString == '_') ++hostnameString;

  // Find the : after the hostname.
  const char* p = hostnameString;
  while ((p - hostnameString < 4096) && *p && ':' != *p)
  {
      ++p;
  }

  if (*p != ':')
  {
      return PkDspyErrorBadParams;
  }

  // Copy out the hostname, null terminate it
  memcpy(hostName, hostnameString, p - hostnameString);
  hostName[p - hostnameString] = '\0';

  // Get the port number as a number.
  char* dummy;
  *portNumber = strtoul(p+1, &dummy, 0);

  *portNumber += 100;

  return PkDspyErrorNone;
}

bool PRManKatanaDisplayDriverHelper::haveObjectForFrame(int frameID)
{
  std::map<int, FnKat::NewFrameMessage* >::iterator it;

  it = _frameMap.find(frameID);

  return (it != _frameMap.end());
}

void PRManKatanaDisplayDriverHelper::addNewFrameObject(int frameID, FnKat::NewFrameMessage* frame)
{
  // Add the frame to the _frameMap...
  _frameMap[frameID] = frame;

  // Add the frame to the _channelMap.
  _channelMap[frame] = 0;
}

FnKat::NewFrameMessage* PRManKatanaDisplayDriverHelper::getFrameObject(int frameID)
{
  return _frameMap[frameID];
}

int PRManKatanaDisplayDriverHelper::getNextChannelID(FnKat::NewFrameMessage *frame)
{
  // First find the std::set for the frame...
  std::map<FnKat::NewFrameMessage*, int>::iterator it = _channelMap.find(frame);

  // Return a new channel ID
  return ++(it->second);
}

/*******************************************************************************
 *                      END RENDERMAN DISPLAY DRIVER HELPER
 ******************************************************************************/

/**************************************
 * EXTERNAL ENTRY POINT DspyImageOpen *
 **************************************/
extern "C" PtDspyError DspyImageOpen
(
    PtDspyImageHandle* handle_p,
    const char* drivername,
    const char* filename,
    int width, int height,
    int paramCount,
    const UserParameter* parameters,
    int formatCount,
    PtDspyDevFormat* format,
    PtFlagStuff* flagstuff
)
{
#ifdef DRIVER_DEBUG
  std::cerr << "Entering DspyImageOpen MultiresDisplay Driver" << std::endl;
  std::cerr << "drivername: " << drivername << ", filename: " << filename << std::endl;
  std::cerr << "width: " << width << ", height: " << height << ", paramCount: " << paramCount << std::endl;
  std::cerr << "formatCount: " << formatCount << std::endl;
#endif

    // Format Renderman parameters in the order we expect to receive them.
    if ((0 == width)||(0 == height)||(0 == formatCount))
        return PkDspyErrorBadParams;

    // Ask for everything in float
    for (int formatIndex = 0; formatIndex < formatCount; ++formatIndex)
        format[formatIndex].type = PkDspyFloat32;
    flagstuff->flags |= PkDspyFlagsWantsEmptyBuckets;

    // Set up the Image structure that will be passed in each time we get data
    UserImageStruct* imgStruct = new UserImageStruct;
    *handle_p = imgStruct;

    // Check if there is a ":lod:" substring in the filename which will indicate
    // this particular image is actually an lower res version of an existing
    // message
    const char* lodOccurrence =  strstr( filename, ":lod:" );

    if ( !lodOccurrence )
    {
      lodOccurrence =  strstr( filename, "lod:" );

      if ( lodOccurrence )
      {
         int lod;
         sscanf( lodOccurrence, "lod:%d", &lod );

         imgStruct->scale = (int)pow(4.0, lod-1);
      }
//      else
//      {
//        imgStruct->scale = 0;
//      }
    }
    else if ( lodOccurrence )
    {
       int lod;
       sscanf( lodOccurrence, ":lod:%d", &lod );

       imgStruct->scale = (int)pow(4.0, lod-1);
    }

    //Assumes channels are floats
    if(gScaledata == NULL)
    {
      gScaledata = (unsigned char*)malloc ( width * height * sizeof(float) * formatCount );
      memset(gScaledata, 0, width * height * sizeof(float) * formatCount);
    }

    // Connect a socket to the client.    
    PtDspyError result = PkDspyErrorNone;

    // Connect the KatanaPipe to our server...
    char hostName[4096];
    unsigned long portNumber;
    result = PkDspyErrorNone;
    if(PkDspyErrorNone != (result = PRManKatanaDisplayDriverHelper::parseHostName(filename, hostName, &portNumber)))
      return result;
    imgStruct->pipe = FnKat::PipeSingleton::Instance(hostName, portNumber);

    if(imgStruct->pipe->connect())
      return PkDspyErrorUndefined;

    // Get the frameID out of the parameter list.
    int frameID = 0;
    DspyFindIntInParamList("frameID", &frameID, paramCount, parameters);

    // First thing we need to do is check if we have a unique frame ID for
    // this render (remember a frame is made up of many channels)
    FnKat::NewFrameMessage *frameObject = NULL;
    if(PRManKatanaDisplayDriverHelper::Instance()->haveObjectForFrame(frameID) )
    {
      frameObject = PRManKatanaDisplayDriverHelper::Instance()->getFrameObject(frameID);
    }
    else
    {
      // We are going to build one...
      frameObject = new FnKat::NewFrameMessage();

      // Frame Time...
      float frameTime = 0;
      DspyFindFloatInParamList("frameTime", &frameTime, paramCount, parameters);
      frameObject->setFrameTime(frameTime);

      // Frame dimensions
      int nvals = 2;
      int originalDimensions[2];
      originalDimensions[0] = 0;
      originalDimensions[1] = 0;
      DspyFindIntsInParamList("OriginalSize", &nvals, originalDimensions, paramCount, parameters);
      frameObject->setFrameDimensions(originalDimensions[0], originalDimensions[1]);

      // Origin is (0,0) - not sure how much we need this?
      frameObject->setFrameOrigin(0, 0);

      // Frame Name...
      char* nameStr = NULL;
      DspyFindStringInParamList("frameName", &nameStr, paramCount, parameters);

      // Version 1.0 of the RDP requires the frame name field is encoded using
      // the following format...
      std::string legacyEncoding;
      if(nameStr)
      {
        FnKat::encodeLegacyName(nameStr, frameID, legacyEncoding);
        frameObject->setFrameName(legacyEncoding);
      }

      // Store the frame in the map...
      PRManKatanaDisplayDriverHelper::Instance()->addNewFrameObject(frameID, frameObject);

      // Send the frame down the pipe...
      imgStruct->pipe->send(*frameObject);
    }

    // We need to check here if we are a reduced resolution version of the main
    // channel, so, first take a look if we are !lod, if we are, create a
    // NewChannelMessage, otherwise, retrieve the NewChannelMessage for the
    // frameID and store it it the imagemap.
    if(!lodOccurrence)
    {
      // Now we need to create a channel that will hold the data...
      int channelID = PRManKatanaDisplayDriverHelper::Instance()->getNextChannelID(frameObject);
      FnKat::NewChannelMessage *channelObject = new FnKat::NewChannelMessage(*frameObject);

      // Set channelID
      channelObject->setChannelID((FnKat::uint16_t)channelID);

      // Origin of pixels relative to frame.
      int nvals = 2;
      int origin[2];
      origin[0] = 0;
      origin[1] = 0;
      DspyFindIntsInParamList("origin", &nvals, origin, paramCount, parameters);

      channelObject->setChannelOrigin(origin[0], origin[1]);

      // Set the height and width
      channelObject->setChannelDimensions(channelObject->channelXOrigin() + width,
                                          frameObject->frameHeight() - channelObject->channelYOrigin());

      // Readjust the origin for renderman...
      channelObject->setChannelOrigin(channelObject->channelXOrigin(),
                                      channelObject->channelHeight() - height);

      // Set sample rate.
      nvals = 2;
      float sampleRate[2];
      sampleRate[0] = 1.0;
      sampleRate[1] = 1.0;
      DspyFindFloatsInParamList("sampleRate", &nvals, sampleRate, paramCount, parameters);
      channelObject->setSampleRate(sampleRate);

      // Data Size... everything is 32 bit float, so multiply by num values
      channelObject->setDataSize(4 * formatCount);

      // Channel Name...
      char* nameStr = NULL;
      DspyFindStringInParamList("frameName", &nameStr, paramCount, parameters);
      // Version 1.0 of the RDP requires the channel name field is encoded using
      // the following format...
      std::string legacyEncoding;
      if(nameStr)
      {
        FnKat::encodeLegacyName(nameStr, frameID, legacyEncoding);
        channelObject->setChannelName(legacyEncoding);
      }

      // Pack the channel down the pipe to and save to our image structure.
      imgStruct->pipe->send(*channelObject);
      imgStruct->channel = channelObject;
      gChannelMsg = channelObject;
      PRManKatanaDisplayDriverHelper::Instance()->addChannelObject(filename, channelObject);
    }
    else
    {
      std::string filesearch(filename);
      std::string highresImageName = filesearch.substr(0, filesearch.find(":lod"));

      // Find the NewChannelMessage and store this in the imgStruct.
      //imgStruct->channel = PRManKatanaDisplayDriverHelper::Instance()->getChannelObjectFor(highresImageName);
      imgStruct->channel = gChannelMsg;
    }

    return PkDspyErrorNone;
}


/**************************************
 * EXTERNAL ENTRY POINT DspyImageData *
 **************************************/
extern "C" PtDspyError DspyImageData(
    PtDspyImageHandle handle,
    int xmin,
    int xmax_plusone,
    int ymin,
    int ymax_plusone,
    int entrysize,
    const unsigned char *data
) 
{

#ifdef DRIVER_DEBUG
  //std::cerr << "Entering DspyImageData" << std::endl;
#endif

    UserImageStruct* imgStruct = reinterpret_cast<UserImageStruct*>(handle);

    // Compute the size of the block,
    int blockW = xmax_plusone - xmin;
    int blockH = ymax_plusone - ymin;

    int scaleblockW = blockW*imgStruct->scale;
    int scaleblockH = blockH*imgStruct->scale;

    size_t blockSize = static_cast<size_t>(scaleblockW * scaleblockH * entrysize);

    //data assumed to point at RGBA float data
    unsigned char *scaledata = (unsigned char *)data;
    if ( imgStruct->scale > 1) {
        scaledata = gScaledata;
        for ( int y = 0; y < blockH; y++ ) {
            for ( int x = 0; x < blockW; x++ ) {
                int X = x*imgStruct->scale;
                for ( int j = 0; j < imgStruct->scale; j++ ) {
                    int Y = (y*imgStruct->scale+j)*scaleblockW;
                    for ( int i = 0; i < imgStruct->scale; i++ ) {
                        memcpy(&scaledata[(X+i+Y)*16],&data[(x+y*blockW)*16],16);
                    }
                }
            }
        }
    }

    if(imgStruct->channel)
    {
      FnKat::DataMessage *dataMsg = new FnKat::DataMessage(*(imgStruct->channel));
      dataMsg->setStartCoordinates(xmin*imgStruct->scale, ymin*imgStruct->scale);
      dataMsg->setDataDimensions(( dataMsg->xMin() + scaleblockW - xmin * imgStruct->scale ), ( dataMsg->yMin() + scaleblockH - ymin * imgStruct->scale ));
      dataMsg->setData(scaledata, blockSize );
      dataMsg->setByteSkip(entrysize);

      //Pack it down the pipe.
      imgStruct->pipe->send(*dataMsg);

      delete dataMsg;
      dataMsg = NULL;
    }

    return PkDspyErrorNone;
}


/***************************************
 * EXTERNAL ENTRY POINT DspyImageClose *
 ***************************************/
extern "C" PtDspyError DspyImageClose
(
    PtDspyImageHandle handle
)
{
#ifdef DRIVER_DEBUG
  std::cerr << "Entering DspyImageClose." << std::endl;
#endif

  // Close the socket and delete the user structure.
  if (0 != handle)
  {
      UserImageStruct* imgStruct = reinterpret_cast<UserImageStruct*>(handle);

      // We are done sending data to this channel so let Katana clean up...
      imgStruct->pipe->flushPipe(*imgStruct->channel);
      imgStruct->pipe->closeChannel(*imgStruct->channel);

      if(gScaledata)
      {
        free( gScaledata );
        gScaledata = 0;
      }

      delete imgStruct;
  }
    return PkDspyErrorNone;
}


/***************************************
 * EXTERNAL ENTRY POINT DspyImageQuery *
 ***************************************/
 
/* My understanding is that this is an archaic call,
   not really useful in a modern context.
   It has to be here, but it doesn't really need to do
   anything useful.
*/
extern "C" PtDspyError DspyImageQuery
(
    PtDspyImageHandle handle,
    PtDspyQueryType querytype,
    int datalen,
    void* data
) 
{
#ifdef DRIVER_DEBUG
  std::cerr << "Entering DspyImageQuery." << std::endl;
#endif

    if (0 >= datalen || 0 == data)
        return PkDspyErrorBadParams;

    switch (querytype)
    {
        case PkOverwriteQuery:
        {
            PtDspyOverwriteInfo overwriteInfo;
            if (size_t(datalen) > sizeof(overwriteInfo))
                datalen = sizeof(overwriteInfo);
            overwriteInfo.overwrite = 1;
            overwriteInfo.interactive = 0;
            memcpy(data, &overwriteInfo, datalen);
            return PkDspyErrorNone;
        }
        
        case PkSizeQuery:
        {
            PtDspySizeInfo sizeInfo;

            if (size_t(datalen) > sizeof(sizeInfo))
                datalen = sizeof(sizeInfo);

            sizeInfo.width = 0;
            sizeInfo.height = 0;
            sizeInfo.aspectRatio = 1.0f;

            memcpy(data, &sizeInfo, datalen);
            return PkDspyErrorNone;;
        }
        case PkMultiResolutionQuery:
        {
            PtDspyMultiResolutionQuery multiRes;

            if (size_t(datalen) > sizeof(multiRes))
                datalen = sizeof(multiRes);

            multiRes.supportsMultiResolution = 1;

            memcpy(data, &multiRes, datalen);
            return PkDspyErrorNone;;
        }
        default:
        {
            return PkDspyErrorUnsupported;
        }
    }
}
