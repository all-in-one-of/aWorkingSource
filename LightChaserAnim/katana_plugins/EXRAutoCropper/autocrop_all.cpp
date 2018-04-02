/*
*/

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <stdio.h>
#include <string>
#include <vector>


 #include <ImfChannelList.h>
#include <half.h>
#include <ImfArray.h>
#include <ImfOutputFile.h>

#include "exr_io.h"

using namespace std;

static void usage(const char * const program) {
    fprintf(stderr, "usage: %s <input.exr> <output.exr>\n", program);
    fprintf(stderr, "\t Auto crop EXR file into a new! \n");
    exit(1);
}

bool endswithdota(char *filename)
{
    bool result = false;
    std::string source = filename;
    if ( source.length() < 2)
    {
        if ( source == "A" )
            result = true;
        else result = false;
    }
    else
    {
        if(source.substr( source.length() - 2 ) == ".A")
        {
            // match
            result = true;
        }        
    }
    return result;
}


int main(int argc, char *argv[]) 
{
    const char *outfile = NULL;
    const char *infile = NULL;

    if (argc == 1) usage(argv[0]);
    for (int i = 1; i < argc; ++i) {
        if (!infile)
            infile = argv[i];
        else if (!outfile)
            outfile = argv[i];
        else
            usage(argv[0]);
    }
    printf("[Start Process] read file from : %s \n", infile);

    Array2D<half> rPixels;
    Array2D<half> gPixels;
    Array2D<half> bPixels;
    Array2D<float> aPixels;
    Box2i dataWindow;
    Box2i displayWindow;
    bool hasAlpha = true;
    bool isCorpped = false;



    dataWindow = ReadDataWindow(infile);
    displayWindow = ReadDisplayWindow(infile);
    printf("[Data Window] min x:%i, min y:%i, max x:%i, max y: %i \n", dataWindow.min.x, dataWindow.min.y, dataWindow.max.x, dataWindow.max.y);
    printf("[Display Window] min x:%i, min y:%i, max x:%i, max y: %i \n", displayWindow.min.x, displayWindow.min.y, displayWindow.max.x, displayWindow.max.y);

    int resolution[2];
    int minx=displayWindow.max.x,miny=displayWindow.max.y,maxx=0,maxy=0;


    vector<string> pixelNameSet;
    vector<string> alphaNameSet;

    Array < Array2D<half> > pixelSet;
    Array < Array2D<float> > alphaSet;
    int width,height;
    // Read EXR
    try{
    InputFile inputFile (infile);
    Box2i dw = inputFile.header().dataWindow();
    Box2i dsw = inputFile.header().displayWindow();
    width  = dsw.max.x - dsw.min.x + 1;
    height = dsw.max.y - dsw.min.y + 1;
    resolution[0] = width;
    resolution[1] = height;
    rPixels.resizeErase (height, width);
    gPixels.resizeErase (height, width);
    bPixels.resizeErase (height, width);
    aPixels.resizeErase (height, width);
    FrameBuffer readFrameBuffer;
 
    if (dw == dsw)
        isCorpped = false;
    else
        isCorpped = true;
    hasAlpha = true;


    const ChannelList &channels = inputFile.header().channels();

    for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
    {
        // const Channel &channel = i.channel();
        // printf("[Channel Name] : %s \n", i.name());
        if ( !endswithdota((char*) i.name()) )
            pixelNameSet.push_back(string(i.name()));
        else
            alphaNameSet.push_back(string(i.name()));
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

    // read alpha channel
    alphaSet.resizeErase(alphaNameSet.size());
    for (unsigned int it = 0; it < alphaNameSet.size(); ++it)
    {
        // printf("read alpha channel: %s \n", alphaNameSet[it].c_str());
        alphaSet[it].resizeErase (height, width);
        readFrameBuffer.insert (alphaNameSet[it],
                            Slice (FLOAT,
                                   (char *) (&alphaSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (alphaSet[it][0][0]) * 1,
                                   sizeof (alphaSet[it][0][0]) * width,
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


    assert(hasAlpha);

    for (int x = 0; x < resolution[0]; ++x)
    {
        for (int y = 0; y < resolution[1]; ++y)
        {
            // printf("---------------------------------------\nx=%i,y=%i \n",x,y);
            if (alphaSet[0][y][x] != 0)
            {
                // printf("alpha %f \n",aPixels[y][x]);
                minx = std::min(minx, x);
                maxx = std::max(maxx, x);
                miny = std::min(miny, y);
                maxy = std::max(maxy, y);
            }
        }
    }
    // return 0;

    if (!isCorpped)
    {
        printf("[Result Data Window] min x :%i,min y :%i,max x :%i,max y: %i \n", minx, miny, maxx, maxy);
        Box2i resultDatawindow(V2i(minx,miny), V2i(maxx, maxy));
        // Box2i resultDatawindow(V2i(500,500), V2i(1000, 1000));
        // writeChannelRGBA(outfile,rPixels,gPixels,bPixels,aPixels,resultDatawindow,displayWindow,resolution[0], resolution[1]);

        // Write EXR
        Header header (resolution[0], resolution[1]);

        width = resolution[0];
        height = resolution[1];
        header.dataWindow() = resultDatawindow;
        header.displayWindow() = displayWindow;

        FrameBuffer writeFrameBuffer;

        // write RGB HALF data
        for (unsigned int it = 0; it < pixelNameSet.size(); ++it)
        {
            // printf("write RGB channel: %s \n", pixelNameSet[it].c_str());
            header.channels().insert (pixelNameSet[it], Channel (HALF));

            writeFrameBuffer.insert (pixelNameSet[it],
                   Slice (HALF, 
                      (char *) &pixelSet[it][0][0],
                      sizeof (pixelSet[it][0][0]),
                      sizeof (pixelSet[it][0][0]) * width,
                      1,  
                      1)
                  );
        }

        // write alpha FLOAT data
        for (unsigned int it = 0; it < alphaNameSet.size(); ++it)
        {
            string alphaName = alphaNameSet[it];
            header.channels().insert (alphaName, Channel (FLOAT));
            // printf("write alpha channel: %s \n", alphaName.c_str());
            writeFrameBuffer.insert (alphaName,
                   Slice (FLOAT, 
                      (char *) &alphaSet[it][0][0],
                      sizeof (alphaSet[it][0][0]),
                      sizeof (alphaSet[it][0][0]) * width,
                      1,  
                      1)
                  );

        }

        OutputFile outputFile (outfile, header);

        outputFile.setFrameBuffer (writeFrameBuffer);
        printf("[Write EXR] save file to :%s \n", outfile);
        outputFile.writePixels (resultDatawindow.max.y - resultDatawindow.min.y + 1);

        return 0;
    }
    else
        printf("[Warning] file already croped: %s \n", infile);

    return 0;
}