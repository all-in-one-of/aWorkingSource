// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <Python.h>

#include <ri.h>
#include <rx.h>
#include <dlfcn.h>
#include <stdio.h>

extern "C" RtPointer ConvertParameters(RtString paramstr);
extern "C" RtVoid Subdivide(RtPointer data, RtFloat detail);
extern "C" RtVoid Free(RtPointer data);

int RxOption (const char *name, void *result, int resultlen, RxInfoType_t *resulttype, int *resultcount)
{
    return -1;
}

int main(int argc, char *argv[])
{
    RiBegin(RI_NULL);
    Subdivide(ConvertParameters("-script ./goscript -fullRecurse"), 0.0);
    RiEnd();

    return 0;
}
