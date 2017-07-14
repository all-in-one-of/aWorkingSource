// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>
#include <ri.h>
#include <rx.h>
#include <pystring/pystring.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * These are the external entry points to the Katana procedural which
 * call through to the KatanaProcedural namespace. This is because we
 * must have Subdivide in an internal namespace to avoid clashes with
 * global symbols.
*/

#ifdef _WIN32
#define FN_FUNCTIONEXPORT __declspec(dllexport)
#else
#define FN_FUNCTIONEXPORT __attribute__ ((visibility("default")))
#endif


extern "C" FN_FUNCTIONEXPORT RtPointer
ConvertParameters( RtString paramstr )
{
    return PRManProcedural::ConvertParameters( paramstr );
}

extern "C" FN_FUNCTIONEXPORT RtVoid
Subdivide( RtPointer data, RtFloat detail )
{
    PRManProcedural::Subdivide( data, detail );
}

extern "C" FN_FUNCTIONEXPORT RtVoid Free( RtPointer data )
{
    PRManProcedural::Free( data );
}

