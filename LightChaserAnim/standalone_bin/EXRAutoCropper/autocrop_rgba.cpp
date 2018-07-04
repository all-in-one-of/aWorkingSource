/*
*/

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "exr_io.h"

static void usage(const char * const program) {
    fprintf(stderr, "usage: %s <input.exr> <output.exr>\n", program);
    fprintf(stderr, "\t Auto crop EXR file into a new! \n");
    exit(1);
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

    float *image;
    int resolution[2];
    Box2i datawindow;
    Box2i displaywindow;
    bool hasAlpha;
    bool isCorpped;
    if (!ReadEXR2(infile, image, resolution[0], resolution[1], hasAlpha, isCorpped)) {
        printf("[Error] couldn't read image %s\n", infile);
        return 1;
    }
    assert(hasAlpha);
    datawindow = ReadDataWindow(infile);
    displaywindow = ReadDisplayWindow(infile);
    printf("[Data Window] min x:%i, min y:%i, max x:%i, max y: %i \n", datawindow.min.x, datawindow.min.y, datawindow.max.x, datawindow.max.y);
    printf("[Display Window] min x:%i, min y:%i, max x:%i, max y: %i \n", displaywindow.min.x, displaywindow.min.y, displaywindow.max.x, displaywindow.max.y);

    int id = 0;
    int minx=displaywindow.max.x,miny=displaywindow.max.y,maxx=0,maxy=0;
    const int pre_pixel = displaywindow.max.x + 1;
    printf("[Pixel Number In Line] pre_pixel : %i \n", pre_pixel);
    // Loops in all pixel to find data window
    for (int i = 0; i < 4*resolution[0]*resolution[1]; i += 4) 
    {
        int height = int(std::floor(id/(pre_pixel)));
        int width =  int(id%pre_pixel);
        float alpha = image[i+3];
        if (alpha != 0)
        {
            minx = std::min(minx, width);
            maxx = std::max(maxx, width);
            miny = std::min(miny, height);
            maxy = std::max(maxy, height);
        }
        // printf("id : %i , width : %i, height : %i \n", id, height, width);
        // printf("-- Color (%f,%f,%f) Alpha(%f) \n", image[i], image[i+1], image[i+2], image[i+3]);
        id ++;
    }
    if (!isCorpped)
    {
        printf("[Result Data Window] min x :%i,min y :%i,max x :%i,max y: %i \n", minx, miny, maxx, maxy);
        Box2i resultDatawindow(V2i(minx,miny), V2i(maxx, maxy));
        printf("[Write EXR] save file to :%s \n", outfile);
        WriteEXR2(outfile, image, resolution[0], resolution[1],resultDatawindow);        
    }
    else
        printf("[Warning] file already croped: %s \n", infile);
    return 0;
}