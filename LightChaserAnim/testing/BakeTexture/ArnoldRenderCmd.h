#pragma once

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>

class CArnoldRenderCmd
   :	public MPxCommand
{

public:

   static void* creator()
   {
      return new CArnoldRenderCmd();
   }

   static MSyntax newSyntax();

   MStatus doIt(const MArgList& argList);

}; // class CArnoldRenderCmd
