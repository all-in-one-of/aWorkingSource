// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <ai.h>

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <sstream>

#include <netdb.h>
#include <errno.h>
#include <map>

//#define DRIVER_DEBUG 1
#ifdef DRIVER_DEBUG
  #include <iostream>
#endif

#include <FnDisplayDriver/FnKatanaDisplayDriver.h>
namespace FnKat = Foundry::Katana;

namespace {

/*******************************************************************************
 *                      ARNOLD DISPLAY DRIVER HELPER
 * This is basically a helper class that helps us manage Frame & Channels and
 * a couple of utility functions.
 ******************************************************************************/
class ArnoldKatanaDisplayDriverHelper
{
public:
  static int parseHostName(const char* hostnameString, char* hostName, unsigned long* portNumber);

  static ArnoldKatanaDisplayDriverHelper* Instance();
  static void Destroy();

  // Do we already have a NewFrameMessage object for the frameID?
  bool haveObjectForFrame(int frameID);
  // Add an NewFrameMessage object for the given frame ID
  void addNewFrameObject(int frameID, FnKat::NewFrameMessage* frame);
  // Get the NewFrameMessage object for a given frameID
  FnKat::NewFrameMessage* getFrameObject(int frameID);

  // Get a channelID for the frame...
  int getNextChannelID(FnKat::NewFrameMessage* frame);

private:
  ArnoldKatanaDisplayDriverHelper() : _frameMap(), _channelMap() {};
  ArnoldKatanaDisplayDriverHelper(const ArnoldKatanaDisplayDriverHelper& rhs);
  ~ArnoldKatanaDisplayDriverHelper() {};

  static ArnoldKatanaDisplayDriverHelper* _instance;

  std::map<int, FnKat::NewFrameMessage* > _frameMap;
  std::map<FnKat::NewFrameMessage*, int > _channelMap;
};

ArnoldKatanaDisplayDriverHelper* ArnoldKatanaDisplayDriverHelper::_instance = NULL;

ArnoldKatanaDisplayDriverHelper* ArnoldKatanaDisplayDriverHelper::Instance()
{
  if(!_instance)
    _instance = new ArnoldKatanaDisplayDriverHelper();

  return _instance;
}

void ArnoldKatanaDisplayDriverHelper::Destroy()
{
  if(!_instance)
  {
    delete _instance;
    _instance = NULL;
  }
}

int ArnoldKatanaDisplayDriverHelper::parseHostName(const char* hostnameString, char* hostName, unsigned long* portNumber)
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
      return -1;
  }

  // Copy out the hostname, null terminate it
  memcpy(hostName, hostnameString, p - hostnameString);
  hostName[p - hostnameString] = '\0';

  // Get the port number as a number.
  char* dummy;
  *portNumber = strtoul(p+1, &dummy, 0);

  *portNumber += 100;

  return 0;
}

bool ArnoldKatanaDisplayDriverHelper::haveObjectForFrame(int frameID)
{
  std::map<int, FnKat::NewFrameMessage* >::iterator it;

  it = _frameMap.find(frameID);

  return (it != _frameMap.end());
}

void ArnoldKatanaDisplayDriverHelper::addNewFrameObject(int frameID, FnKat::NewFrameMessage* frame)
{
  // Add the frame to the _frameMap...
  _frameMap[frameID] = frame;

  // Add the frame to the _channelMap.
  _channelMap[frame] = 0;
}

FnKat::NewFrameMessage* ArnoldKatanaDisplayDriverHelper::getFrameObject(int frameID)
{
  return _frameMap[frameID];
}

int ArnoldKatanaDisplayDriverHelper::getNextChannelID(FnKat::NewFrameMessage *frame)
{
  // First find the std::set for the frame...
  std::map<FnKat::NewFrameMessage*, int>::iterator it = _channelMap.find(frame);

  // Return a new channel ID
  return ++(it->second);
}

/*******************************************************************************
 *                      END ARNOLD DISPLAY DRIVER HELPER
 ******************************************************************************/

struct ViewInfo
{
    ViewInfo() :
        pipe(0),
        channel(0),
        socket(0),
        xo(0),
        yo(0),
        pixels() 
    {
        // first time through - set an invalid region for both
        previous_disp_win.minx = previous_data_win.minx = -1;
        previous_disp_win.maxx = previous_data_win.maxx = -1;
        previous_disp_win.miny = previous_data_win.miny = -1;
        previous_disp_win.maxy = previous_data_win.maxy = -1;
    }

    bool regionMatch(const AtBBox2& disp_win, const AtBBox2& data_win) const 
    {
        return memcmp(&disp_win, &previous_disp_win, sizeof(AtBBox2)) == 0 &&
               memcmp(&data_win, &previous_data_win, sizeof(AtBBox2)) == 0;
    }

    FnKat::KatanaPipe *pipe;
    FnKat::NewChannelMessage *channel;

    int socket;
    AtBBox2 previous_disp_win, previous_data_win;
    int xo, yo; // image origin (for render region adjustment)
    std::vector<float> pixels;
};

/**
 * 
 * Sends a region of pixels to the screen. The coordinates describe the region:
 *    [x0, x1) x [y0, y1)
 * 
 * FIXME: doesn't work when not inline !!??
 */
inline void sendRegion(int x0, int y0, int x1, int y1, const std::vector<float>& pixels, FnKat::NewChannelMessage *c, FnKat::KatanaPipe *pipe ) 
{
    if (x1 <= x0 || y1 <= y0 || ((size_t) 4 * (x1 - x0) * (y1 - y0)) > pixels.size()) 
    {
        AiMsgWarning("[KatanaDriver] invalid region: [%d, %d) x [%d, %d)", x0, x1, y0, y1);
        return; // invalid region
    }

    FnKat::DataMessage *dataMsg = new FnKat::DataMessage(*c);
    dataMsg->setStartCoordinates(x0, y0);
    dataMsg->setDataDimensions((x1 - x0), (y1 - y0));
    dataMsg->setData(&pixels[0], ((x1 - x0) * (y1 - y0) * sizeof(float) * 4) );
    dataMsg->setByteSkip(sizeof(float) * 4);

    pipe->send(*dataMsg);

    delete dataMsg;
    dataMsg = NULL;
}

#define _port                (params[0].STR  )
#define _frame_id            (params[1].INT  )
#define _frame_time          (params[2].FLT  )
#define _frame_name          (params[3].STR  )
#define _bucket_corners      (params[4].BOOL )
#define _allowIDPass         (params[5].BOOL )
#define _frame_origin        (params[6].ARRAY)
#define _display_window_size (params[7].ARRAY)

AI_DRIVER_NODE_EXPORT_METHODS(DriverKatanaMtd);

node_parameters
{
    AiParameterSTR  ("port"               , "localhost:15900"); // default
    AiParameterINT  ("frame_id"           , 1);
    AiParameterFLT  ("frame_time"         , 1);
    AiParameterSTR  ("frame_name"         , "Arnold");
    AiParameterBOOL ("bucket_corners"     , true);
    AiParameterBOOL ("allow_id_pass"      , false);
    AiParameterARRAY("frame_origin"       , AiArray(2, 1, AI_TYPE_INT, 0, 0));
    AiParameterARRAY("display_window_size", AiArray(2, 1, AI_TYPE_INT, 0, 0));
}

node_initialize
{
    if (AiNodeGetLocalData(node))
      return; // don't re-init during progressive rendering
    AiDriverInitialize(node, false, (void*) new ViewInfo());
}

driver_extension
{
   return NULL;
}

driver_supports_pixel_type
{
   switch (pixel_type) {
      case AI_TYPE_INT:
      case AI_TYPE_FLOAT:
      case AI_TYPE_RGBA:
         return true;
      default:
         return false;
   }
}

driver_open
{
    const AtParamValue *params = AiNodeGetParams(node);
    ViewInfo*   ptr = (ViewInfo*)AiDriverGetLocalData(node);
    if (!ptr) return; // error?

    if (!ptr->regionMatch(display_window, data_window))
    {
        ptr->previous_disp_win = display_window;
        ptr->previous_data_win = data_window;
    }
    else if (ptr->regionMatch(display_window, data_window))
        return;

    ptr->xo = data_window.minx;
    ptr->yo = data_window.miny;

    ptr->pixels.resize(4 * bucket_size * bucket_size);

    // Connect to the server.
    char hostName[4096];
    unsigned long portNumber;
    if(ArnoldKatanaDisplayDriverHelper::parseHostName(_port, hostName, &portNumber))
        return;
    ptr->pipe = FnKat::PipeSingleton::Instance(hostName, portNumber);

    if(ptr->pipe->connect())
        return;

    int displayWindow[] = { 0,
                            0,
                            AiArrayGetInt(_display_window_size, 0),
                            AiArrayGetInt(_display_window_size, 1)
    };

    // Check if we have a unique frame ID for this render...
    FnKat::NewFrameMessage *frameObject = NULL;
    if(ArnoldKatanaDisplayDriverHelper::Instance()->getFrameObject(_frame_id))
    {
        frameObject = ArnoldKatanaDisplayDriverHelper::Instance()->getFrameObject(_frame_id);
    }
    else
    {
        // We should build one...
        frameObject = new FnKat::NewFrameMessage();

        // Frame time..
        frameObject->setFrameTime(_frame_time);

        // Frame dimensions
        frameObject->setFrameDimensions(displayWindow[2], displayWindow[3]);

        // Origin
        frameObject->setFrameOrigin(0, 0);

        // Frame Name
        // Version 1.0 of the RDP requires the frame name field is encoded using
        // the following format...
        std::string legacyEncoding;
        FnKat::encodeLegacyName(_frame_name, _frame_id, legacyEncoding);
        frameObject->setFrameName(legacyEncoding);

        // Store in the map
        ArnoldKatanaDisplayDriverHelper::Instance()->addNewFrameObject(_frame_id, frameObject);

        // Send down the pipe...
        ptr->pipe->send(*frameObject);
    }

    // Now we need to create a channel that will hold the data...
    int channelID = ArnoldKatanaDisplayDriverHelper::Instance()->getNextChannelID(frameObject);
    FnKat::NewChannelMessage *channelObject = new FnKat::NewChannelMessage(*frameObject);

    // Set channelID
    channelObject->setChannelID((FnKat::uint16_t)channelID);

    // Origin of pixels relative to frame.
    int frameOrigin[] = { AiArrayGetInt(_frame_origin, 0),
                          AiArrayGetInt(_frame_origin, 1)
    };

    channelObject->setChannelOrigin(frameOrigin[0],
                                    displayWindow[3] - frameOrigin[1] - (data_window.maxy - data_window.miny + 1));

    // Set height and width...
    channelObject->setChannelDimensions(frameOrigin[0] + (data_window.maxx - data_window.minx + 1),
                                        displayWindow[3] - frameOrigin[1]);

    // Set the sample rate
    float sampleRate[2];
    sampleRate[0] = 1.0;
    sampleRate[1] = 1.0;
    channelObject->setSampleRate(sampleRate);

    // Data Size... everything is 32 bit float, so multiply by num values...
    channelObject->setDataSize(16);

    // Channel Name...
    // Version 1.0 of the RDP requires the channel name field is encoded using
    // the following format...
    std::string legacyEncoding;
    FnKat::encodeLegacyName(_frame_name, _frame_id, legacyEncoding);
    channelObject->setChannelName(legacyEncoding);

    // Pack the channel down the pipe to and save to our image structure.
    ptr->pipe->send(*channelObject);
    ptr->channel = channelObject;
}

driver_needs_bucket
{
    // All the buckets must be sent back to Katana.
    return true;
}

driver_process_bucket
{
    // There is no need to process the pixels.
}

driver_prepare_bucket
{
    const AtParamValue* params = AiNodeGetParams(node);
    if (!_bucket_corners) return; // we're in a hurry
    ViewInfo* ptr = (ViewInfo*)AiDriverGetLocalData(node);

    if (!ptr ) return;

    // pick a color based on thread ID
    const float c1 = 0.10f;
    const float c0 = 0.05f;

    float R, G, B;
    switch (tid % 6) 
    {
        case 0: R = c1; G = c0; B = c0; break;
        case 1: R = c0; G = c1; B = c0; break;
        case 2: R = c0; G = c0; B = c1; break;
        case 3: R = c1; G = c1; B = c0; break;
        case 4: R = c1; G = c0; B = c1; break;
        case 5: R = c0; G = c1; B = c1; break;
        default: R = G = B = c0; break; // ?
    }
    int lineWidthX = std::max(1, (bucket_size_x + 4) / 10);
    int lineWidthY = std::max(1, (bucket_size_y + 4) / 10);
    if (lineWidthX == 1) lineWidthX = bucket_size_x;
    if (lineWidthY == 1) lineWidthY = bucket_size_y;
    int lineWidth = 4 * std::max(lineWidthX, lineWidthY);
    for (int j = 0, i4 = 0; j < lineWidth; j++, i4 += 4) 
    {
        // fill with line color
        ptr->pixels[i4 + 0] = 1;
        ptr->pixels[i4 + 1] = R;
        ptr->pixels[i4 + 2] = G;
        ptr->pixels[i4 + 3] = B;
    }
    int x0 = bucket_xo - ptr->xo;
    int y0 = bucket_yo - ptr->yo;

    sendRegion(x0                               , y0,
               x0 + lineWidthX                  , y0 + 1,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0 + bucket_size_x - lineWidthX  , y0,
               x0 + bucket_size_x               , y0 + 1,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0                               , y0 + bucket_size_y - 1,
               x0 + lineWidthX                  , y0 + bucket_size_y,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0 + bucket_size_x - lineWidthX  , y0 + bucket_size_y - 1,
               x0 + bucket_size_x               , y0 + bucket_size_y,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0                               , y0,
               x0 + 1                           , y0 + lineWidthY,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0                               , y0 + bucket_size_y - lineWidthY,
               x0 + 1                           , y0 + bucket_size_y,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0 + bucket_size_x - 1           , y0,
               x0 + bucket_size_x               , y0 + lineWidthY,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);

    sendRegion(x0 + bucket_size_x - 1           , y0 + bucket_size_y - lineWidthY,
               x0 + bucket_size_x               , y0 + bucket_size_y,
               ptr->pixels,
               ptr->channel,
               ptr->pipe);
}

driver_write_bucket
{
    const char*    output_name;
    int          pixel_type;
    const void*  bucket_data;

    ViewInfo* ptr = (ViewInfo*)AiDriverGetLocalData(node);

    if(!ptr) return;

    /*
    * we'll just look at the first layer -- if you want
    * to iterate through all the layers, then change
    * if(){...} to while(){...}
    */
    if (AiOutputIteratorGetNext(iterator, &output_name, &pixel_type, &bucket_data))
    {
    /*
    * write the incoming channel
    */
        switch (pixel_type)
        {
            case AI_TYPE_RGBA:
            {
                for (int j = 0, i4 = 0; j < bucket_size_y; j++)
                {
                    for (int i=0; i < bucket_size_x; i++, i4 += 4)
                    {
                        float valR = ((const float*) bucket_data)[i4 + 0];
                        float valG = ((const float*) bucket_data)[i4 + 1];
                        float valB = ((const float*) bucket_data)[i4 + 2];
                        float valA = ((const float*) bucket_data)[i4 + 3];
                        ptr->pixels[i4 + 0] = valA;
                        ptr->pixels[i4 + 1] = valR;
                        ptr->pixels[i4 + 2] = valG;
                        ptr->pixels[i4 + 3] = valB;
                    }
                }
                break;
            }
            case AI_TYPE_INT:
            {
                //Katana socket expects float RGBA since that's all it ever receives
                //from prman.  Stuff single int value in the alpha channel.
                for (int j=0, i4 = 0, i1 = 0; j < bucket_size_y; j++)
                {
                    for (int i = 0; i < bucket_size_x; i++, i4 += 4, i1++)
                    {
                        ptr->pixels[i4 + 0] = (float) ((const int*) bucket_data)[i1];
                        ptr->pixels[i4 + 1] = 0;
                        ptr->pixels[i4 + 2] = 0;
                        ptr->pixels[i4 + 3] = 0;
                    }
                }
                break;
            }
            case AI_TYPE_FLOAT:
            {
                //Katana socket expects float RGBA since that's all it ever receives
                //from prman. Copy into rgb as if this was a greyscale image (alpha=1)
                for (int j=0, i4 = 0, i1 = 0; j < bucket_size_y; j++)
                {
                    for (int i = 0; i < bucket_size_x; i++, i4 += 4, i1++)
                    {
                        ptr->pixels[i4 + 0] = 1;
                        ptr->pixels[i4 + 1] = (float) ((const float*) bucket_data)[i1];
                        ptr->pixels[i4 + 2] = (float) ((const float*) bucket_data)[i1];
                        ptr->pixels[i4 + 3] = (float) ((const float*) bucket_data)[i1];
                    }
                }
                break;
            }
            default: return; // error checking done by Arnold
        }

        sendRegion(bucket_xo - ptr->xo,
                   bucket_yo - ptr->yo,
                   bucket_xo - ptr->xo + bucket_size_x,
                   bucket_yo - ptr->yo + bucket_size_y,
                   ptr->pixels,
                   ptr->channel,
                   ptr->pipe);
   }
}

driver_close
{
    ViewInfo*   ptr = (ViewInfo*)AiDriverGetLocalData(node);
    AiMsgDebug("[driver_close] Is being called.");

    // flushPipe() provides us with an opportunity to flush the pipe
    // it will block until Katana has received all the image data.
    ptr->pipe->flushPipe(*(ptr->channel));
}

node_update
{
}

node_finish
{
    // We need to instruct Katana to clear up all data structures by sending
    // an EndOfRenderMessage.
    ViewInfo *ptr = (ViewInfo *)AiDriverGetLocalData(node);

    // Tell Katana we have nothing else to send it so it's free to do
    // cleanup.
    ptr->pipe->closeChannel( *(ptr->channel) );

    // Release our local info structure
    delete ptr;

    // release the driver
    AiDriverDestroy(node);
}

} // anonymous namespace

void loadDriverKatana()
{
    AiNodeEntryInstall(AI_NODE_DRIVER, AI_TYPE_UNDEFINED, "driver_katana", __FILE__,
                       static_cast<AtNodeMethods*>(DriverKatanaMtd), AI_VERSION);
}
