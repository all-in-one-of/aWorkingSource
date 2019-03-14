#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>

#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfRgbaFile.h>
#include <half.h>

#include "exrio.h"

using namespace std;
using namespace Imf;
using namespace Imath;

// Get current time, format is HH:mm:ss
const std::string currentTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

static void usage(const char * const program) {
    fprintf(stderr, "usage: %s <input.exr> <output.exr>\n", "OpenEXRScanline");
    fprintf(stderr, "\t\t -h  print help information \n");
    fprintf(stderr, "\t Convert OpenEXR file to scanline type. \n");
    exit(1);
}

int main(int argc, char *argv[]) 
{
    const char *infile = NULL;
    const char *outfile = NULL;
    // const char *resolution = "FLOAT";

    // chasing arguments
    if (argc == 1) usage(argv[0]);
    for (int i = 1; i < argc; i++) {
        if (i > 3)
            printf("Too many arguments!");
        else if (strcmp(argv[i], "-h") == 0)
            usage(argv[0]); 
        else if (!infile)
            infile = argv[i];
        else if (!outfile)
            outfile = argv[i];
        else
            usage(argv[0]);
    }

    printf("[%s] Read file from : %s \n",currentTime().c_str(), infile);


    HeaderData data;
    ReadHeaderData(infile, data);

    Box2i dataWindow = data.dataWindow;
    Box2i displayWindow = data.displayWindow;



    vector<string> floatPixelNameSet;
    Array < Array2D<float> > floatPixelSet;
    vector<string> halfPixelNameSet;
    Array < Array2D<half> > halfPixelSet;
    vector<string> uintPixelNameSet;
    Array < Array2D<uint> > uintPixelSet;


    int width,height;

    // Read EXR
    try{
    InputFile inputFile (infile);
    Box2i dw = inputFile.header().dataWindow();
    Box2i dsw = inputFile.header().displayWindow();
    width  = dsw.max.x - dsw.min.x + 1;
    height = dsw.max.y - dsw.min.y + 1;

    FrameBuffer readFrameBuffer;

    const ChannelList &channels = inputFile.header().channels();

    for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
    {
        switch (i.channel().type){
            case FLOAT:
                floatPixelNameSet.push_back(string(i.name()));
                break;
            case HALF:
                halfPixelNameSet.push_back(string(i.name()));
                break;
            case UINT:
                uintPixelNameSet.push_back(string(i.name()));
                break;
            default:
                floatPixelNameSet.push_back(string(i.name()));
                break;
        }
    }


    // read FLOAT RGB channel
    floatPixelSet.resizeErase (floatPixelNameSet.size());
    for (unsigned int it = 0; it < floatPixelNameSet.size(); ++it)
    {
        floatPixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (floatPixelNameSet[it],
                            Slice (FLOAT,
                                   (char *) (&floatPixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (floatPixelSet[it][0][0]) * 1,
                                   sizeof (floatPixelSet[it][0][0]) * width,
                                   1, 1,
                                   0.0));        
    }

    // read HALF RGB channel
    halfPixelSet.resizeErase (halfPixelNameSet.size());
    for (unsigned int it = 0; it < halfPixelNameSet.size(); ++it)
    {
        halfPixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (halfPixelNameSet[it],
                            Slice (HALF,
                                   (char *) (&halfPixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (halfPixelSet[it][0][0]) * 1,
                                   sizeof (halfPixelSet[it][0][0]) * width,
                                   1, 1,
                                   0.0));        
    }


    // read UINT RGB channel
    uintPixelSet.resizeErase (uintPixelNameSet.size());
    for (unsigned int it = 0; it < uintPixelNameSet.size(); ++it)
    {
        uintPixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (uintPixelNameSet[it],
                            Slice (UINT,
                                   (char *) (&uintPixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (uintPixelSet[it][0][0]) * 1,
                                   sizeof (uintPixelSet[it][0][0]) * width,
                                   1, 1,
                                   0.0));        
    }


    inputFile.setFrameBuffer (readFrameBuffer);
    inputFile.readPixels (dw.min.y, dw.max.y);
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "[Error] unable to read image file \"%s\": %s \n", infile, e.what());
        return 0;
    }


    // write custom attributes
    Header header (height, width);
    header.dataWindow() = dataWindow;
    header.displayWindow() = displayWindow;
    header.compression() = ZIPS_COMPRESSION;
    vector<MapAttrbute>* list = data.get();
    for(vector<MapAttrbute>::iterator it = list->begin(); it != list->end(); ++it)
    {
        if (!it->value().empty())
            header.insert (it->attribute(), StringAttribute (it->value()));
    }



    FrameBuffer writeFrameBuffer;


    // WRITE FLOAT PIXEL DATA
    for (unsigned int it = 0; it < floatPixelNameSet.size(); ++it)
    {
        header.channels().insert (floatPixelNameSet[it], Channel (FLOAT));
        // For smaller memory buffers with room only for the pixels in the data window, 
        // the base, xStride and yStride arguments for the FrameBuffer object's slices 
        // would have to be adjusted accordingly.
        writeFrameBuffer.insert (floatPixelNameSet[it],
               Slice (FLOAT,
                  (char *) (&floatPixelSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (floatPixelSet[it][0][0]) * 1,
                  sizeof (floatPixelSet[it][0][0]) * width,
                  1,
                  1));
    }

    // WRITE HALF PIXEL DATA
    for (unsigned int it = 0; it < halfPixelNameSet.size(); ++it)
    {
        header.channels().insert (halfPixelNameSet[it], Channel (HALF));
        // For smaller memory buffers with room only for the pixels in the data window, 
        // the base, xStride and yStride arguments for the FrameBuffer object's slices 
        // would have to be adjusted accordingly.
        writeFrameBuffer.insert (halfPixelNameSet[it],
               Slice (HALF,
                  (char *) (&halfPixelSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (halfPixelSet[it][0][0]) * 1,
                  sizeof (halfPixelSet[it][0][0]) * width,
                  1,
                  1));
    }

    // WRITE UINT PIXEL DATA
    for (unsigned int it = 0; it < uintPixelNameSet.size(); ++it)
    {
        header.channels().insert (uintPixelNameSet[it], Channel (UINT));
        // For smaller memory buffers with room only for the pixels in the data window, 
        // the base, xStride and yStride arguments for the FrameBuffer object's slices 
        // would have to be adjusted accordingly.
        writeFrameBuffer.insert (uintPixelNameSet[it],
               Slice (UINT,
                  (char *) (&uintPixelSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (uintPixelSet[it][0][0]) * 1,
                  sizeof (uintPixelSet[it][0][0]) * width,
                  1,
                  1));
    }

    OutputFile outputFile (outfile, header);


    outputFile.setFrameBuffer (writeFrameBuffer);
    printf("[%s] Write EXR save file to :%s \n", currentTime().c_str(), outfile);
    outputFile.writePixels (dataWindow.max.y - dataWindow.min.y + 1);

    return 0;
}