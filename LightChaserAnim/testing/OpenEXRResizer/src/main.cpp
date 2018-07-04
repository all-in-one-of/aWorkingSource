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
    fprintf(stderr, "usage: %s <input.exr> <output.exr>\n", "OpenEXRResizer");
    fprintf(stderr, "\t\t -r  factor of scale [2(default), 4, 8] \n");
    fprintf(stderr, "\t\t -h  print help information \n");
    fprintf(stderr, "\t Resize OpenEXR file, for proxy usage! \n");
    exit(1);
}

int main(int argc, char *argv[]) 
{
    const char *infile = NULL;
    const char *outfile = NULL;
    const char *factor = "2";

    // chasing arguments
    if (argc == 1) usage(argv[0]);
    for (int i = 1; i < argc; i++) {
        if (i > 4)
            printf("Too many arguments!");
        else if (strcmp(argv[i], "-r") == 0)
        {
            factor = argv[i + 1];i++;
        }
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

    Array2D<half> rPixels;
    Array2D<half> gPixels;
    Array2D<half> bPixels;
    Array2D<float> aPixels;
    Box2i dataWindow;
    Box2i displayWindow;


    dataWindow = ReadDataWindow(infile);
    displayWindow = ReadDisplayWindow(infile);

    vector<string> pixelNameSet;
    Array < Array2D<half> > pixelSet;
    
    int width,height;
    
    // Read EXR
    try{
    InputFile inputFile (infile);
    Box2i dw = inputFile.header().dataWindow();
    Box2i dsw = inputFile.header().displayWindow();
    width  = dsw.max.x - dsw.min.x + 1;
    height = dsw.max.y - dsw.min.y + 1;
    rPixels.resizeErase (height, width);
    gPixels.resizeErase (height, width);
    bPixels.resizeErase (height, width);
    aPixels.resizeErase (height, width);
    FrameBuffer readFrameBuffer;
 

    const ChannelList &channels = inputFile.header().channels();

    for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
    {
        pixelNameSet.push_back(string(i.name()));
    }

    // read RGB channel
    pixelSet.resizeErase (pixelNameSet.size());
    for (unsigned int it = 0; it < pixelNameSet.size(); ++it)
    {
        // printf("read RGB channel: %s \n", pixelNameSet[it].c_str());
        pixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (pixelNameSet[it],
                            Slice (HALF,
                                   (char *) (&pixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (pixelSet[it][0][0]) * 1,
                                   sizeof (pixelSet[it][0][0]) * width,
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


    // deal with image size
    Array < Array2D<half> > pixelScaledSet;
    pixelScaledSet.resizeErase (pixelNameSet.size());
    dataWindow.min.x = floor(dataWindow.min.x/atoi(factor));
    dataWindow.min.y = floor(dataWindow.min.y/atoi(factor));
    dataWindow.max.x = floor(dataWindow.max.x/atoi(factor));
    dataWindow.max.y = floor(dataWindow.max.y/atoi(factor));
    displayWindow.min.x = floor(displayWindow.min.x/atoi(factor));
    displayWindow.min.y = floor(displayWindow.min.y/atoi(factor));
    displayWindow.max.x = floor(displayWindow.max.x/atoi(factor));
    displayWindow.max.y = floor(displayWindow.max.y/atoi(factor));
    
    height = floor(height/atoi(factor));
    width = floor(width/atoi(factor));
    for (unsigned int it = 0; it < pixelNameSet.size(); ++it)
    {
        pixelScaledSet[it].resizeErase (height, width);
        
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                pixelScaledSet[it][i][j] = pixelSet[it][i*atoi(factor)][j*atoi(factor)];
            }
        }
    }

    // Write EXR
    Header header (height, width);

    header.dataWindow() = dataWindow;
    header.displayWindow() = displayWindow;

    FrameBuffer writeFrameBuffer;

    // write RGB HALF data
    for (unsigned int it = 0; it < pixelNameSet.size(); ++it)
    {
        header.channels().insert (pixelNameSet[it], Channel (HALF));
        // For smaller memory buffers with room only for the pixels in the data window, 
        // the base, xStride and yStride arguments for the FrameBuffer object's slices 
        // would have to be adjusted accordingly.
        writeFrameBuffer.insert (pixelNameSet[it],
               Slice (HALF,
                  (char *) (&pixelScaledSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (pixelScaledSet[it][0][0]) * 1,
                  sizeof (pixelScaledSet[it][0][0]) * width,
                  1,
                  1));
    }

    OutputFile outputFile (outfile, header);

    outputFile.setFrameBuffer (writeFrameBuffer);
    printf("[%s] Write EXR save file to :%s \n", currentTime().c_str(), outfile);
    outputFile.writePixels (dataWindow.max.y - dataWindow.min.y + 1);

    return 0;
}