#include "ArnoldRenderCmd.h"
#include "scene/MayaScene.h"
#include "render/OutputDriver.h"
#include "utils/FileUtils.h"

#include <ai_msg.h>
#include <ai_universe.h>

#include <maya/M3dView.h>
#include <maya/MArgDatabase.h>
#include <maya/MRenderView.h>
#include <maya/MStringArray.h>
#include <maya/MItDag.h>
#include <maya/MPlug.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MRenderUtil.h>
#include <maya/MFileIO.h>
#include <maya/MFileObject.h>
#include <maya/MFnRenderLayer.h>
#include <maya/MAnimControl.h>
#include <maya/MDagPathArray.h>

#include <sstream>
#include <set>

extern const AtNodeMethods* batch_progress_driver_mtd;

MSyntax CArnoldRenderCmd::newSyntax()
{
   MSyntax syntax;

   syntax.addFlag("b", "batch", MSyntax::kNoArg);
   syntax.addFlag("p", "port", MSyntax::kUnsigned);
   syntax.addFlag("cam", "camera", MSyntax::kSelectionItem);
   syntax.addFlag("w", "width", MSyntax::kUnsigned);
   syntax.addFlag("h", "height", MSyntax::kUnsigned);
   syntax.addFlag("ofn", "origFileName", MSyntax::kString);
   syntax.addFlag("seq", "frameSequence", MSyntax::kString);
   syntax.addFlag("srv", "saveToRenderView", MSyntax::kString);

   return syntax;
}

MStatus CArnoldRenderCmd::doIt(const MArgList& argList)
{
   MStatus status;
   MArgDatabase args(syntax(), argList);
   MDagPath dagPath;

   
   const bool batch = args.isFlagSet("batch") ? true : false;
   const bool sequence = args.isFlagSet("frameSequence") ? true : false;
   const bool multiframe = batch || sequence;

   MCommonRenderSettingsData renderGlobals;
   MRenderUtil::getCommonRenderSettings(renderGlobals);

   /* Ticket #2377 : 
   Commenting this for now, as we don't know if we really want ARV to be used for sequence rendering
   
   if (sequence && !batch)
   {
      // Sequence interactive rendering
      // RenderSession will try to render it with the Arnold RenderView.
      // If it can't it will return false, and we'll keep using Maya's native one
      if (CMayaScene::GetRenderSession()->RenderSequence()) return MS::kSuccess;
   }
   */

   // Rendered camera
   MString camera = "";
   if (!args.isFlagSet("camera"))
   {
      // no camera set on single frame mode, abort
      if (!multiframe) return MS::kFailure;
   }
   else
   {
      camera = args.flagArgumentString("camera", 0);
   }
   // TODO: get the "selected" flag here
   bool exportSelected = !renderGlobals.renderAll;
   MSelectionList selected;
   MSelectionList* selectedPtr = NULL;
   if (exportSelected)
   {
      MGlobal::getActiveSelectionList(selected);
      selectedPtr = &selected;
   }

   int width = args.isFlagSet("width") ? args.flagArgumentInt("width", 0) : -1;
   int height = args.isFlagSet("height") ? args.flagArgumentInt("height", 0) : -1;

   // FIXME: just a fast hack, should rehaul CRenderOptions code
   // and share same proc for ArnoldRenderCmd and ArnoldExportAssCmd
   // TODO : use MString CRenderOptions::VerifyFileName(MString fileName, bool compressed)
   // code to support compressed output filename too
   short renderType = 0;
   bool outputAssBoundingBox = false;
   bool useBinaryEncoding = true;
   bool forceTranslateShadingEngines = false;
   bool progressiveRefinement = true;
   bool exportAllShadingGroups = false;
   bool exportFullPaths = false;
   
   MSelectionList list;
   MObject node;
   list.add("defaultArnoldRenderOptions");
   bool expandProcedurals = false;
   MString kickRenderFlags = "";
   if (list.length() > 0)
   {
      list.getDependNode(0, node);
      MFnDependencyNode fnArnoldRenderOptions(node);
      renderType = fnArnoldRenderOptions.findPlug("renderType").asShort();
      outputAssBoundingBox = fnArnoldRenderOptions.findPlug("outputAssBoundingBox").asBool();
      expandProcedurals = fnArnoldRenderOptions.findPlug("expandProcedurals").asBool();
      kickRenderFlags = fnArnoldRenderOptions.findPlug("kickRenderFlags").asString();
      useBinaryEncoding = fnArnoldRenderOptions.findPlug("binaryAss").asBool();
      forceTranslateShadingEngines = fnArnoldRenderOptions.findPlug("forceTranslateShadingEngines").asBool();
      progressiveRefinement = fnArnoldRenderOptions.findPlug("progressive_rendering").asBool();
      exportAllShadingGroups = fnArnoldRenderOptions.findPlug("exportAllShadingGroups").asBool();
      exportFullPaths = fnArnoldRenderOptions.findPlug("exportFullPaths").asBool();
   }
   if (renderType != MTOA_RENDER_INTERACTIVE)
   {
      // FIXME: actual export code should be shared so we don't have to do this dirty call
      MString cmdStr = "arnoldExportAss";
      if (batch)
      {
         cmdStr += " -b";
      }
      if (exportSelected)
      {
         cmdStr += " -s";
      }
      if (!useBinaryEncoding)
      {
         cmdStr += " -asciiAss";
      }
      if (renderType == MTOA_RENDER_EXPORTASS)
      {         
         if (outputAssBoundingBox)
            cmdStr += " -bb";
      }
      if (expandProcedurals)
         cmdStr += " -ep";
      if (forceTranslateShadingEngines)
      {
         cmdStr += " -forceTranslateShadingEngines";
      }
      if (exportAllShadingGroups)
         cmdStr += " -shg";

      if (exportFullPaths)
         cmdStr += " -fp";

      if (renderGlobals.isAnimated())
      {     
         if (!((renderType == MTOA_RENDER_EXPORTASS_AND_KICK) && !batch))
         {
            float startframe = static_cast<float> (renderGlobals.frameStart.as(MTime::uiUnit()));
            float endframe = static_cast<float> (renderGlobals.frameEnd.as(MTime::uiUnit())); 
            float byframestep = renderGlobals.frameBy;
            cmdStr += " -sf ";
            cmdStr += startframe;
            cmdStr += " -ef ";
            cmdStr += endframe;
            cmdStr += " -fs ";
            cmdStr += byframestep;
         }         
      }
      if (camera != "")
      {
         cmdStr += " -cam " + camera;
      }

      MGlobal::displayInfo("[mtoa] Executing Maya command " + cmdStr);
      MStringArray assFileNames;
      status = MGlobal::executeCommand(cmdStr, assFileNames);
      unsigned int nfiles = assFileNames.length();
      
      
      MString mtoaBinPath = "";
      MString mtoaShaderPath = "";
      const char* mtoaEnvVar = getenv("MTOA_PATH");
      if (mtoaEnvVar)
      {
         mtoaBinPath = mtoaEnvVar;
         mtoaBinPath += "bin/";

         mtoaShaderPath = mtoaEnvVar;
         mtoaShaderPath += "shaders";
      }
      // FIXME eventually use these paths in kickCmd !!
      
      
      if (MStatus::kSuccess == status && nfiles)
      {
         MGlobal::displayInfo("[mtoa] Exported scene to file " + assFileNames[0]);
         if (renderType == MTOA_RENDER_EXPORTASS_AND_KICK)
         {
            // TODO: will only works for single frame, batch render should be used for multiple
            // TODO: might want to remove this as it's a testing implementation and call kick from
            // post render scripts
            MString kickCmd;
            if (batch)
            {
               const unsigned int numAssfiles = assFileNames.length();
               kickCmd = "";
               for (unsigned int i = 0; i < numAssfiles; ++i)
                  kickCmd += "kick -dw -dp \"" + assFileNames[i] + "\";";
            }
            else
            {
               if (!progressiveRefinement)
                  kickRenderFlags += " -dp";
               
#ifdef _WIN32
               kickCmd = "Start kick " + kickRenderFlags + " \"" + assFileNames[0] + "\"";
#else
               kickCmd = "kick " + kickRenderFlags + " \"" + assFileNames[0] + "\" &";
#endif
            }
            // NOTE: must be blocking when in batch mode, non blocking when in interractive mode

            MGlobal::displayInfo("[mtoa] Calling external command " + kickCmd);
            int kickRet = system(kickCmd.asChar());            
            if (kickRet)
            {
               MString msg = "[mtoa] Kick return code : ";
               msg += kickRet;
               MGlobal::displayWarning(msg);
            }

            // TODO : use pykick and MGlobal::executePythonCommandOnIdle to display feedback?
         }
      }
      else
      {
         MGlobal::displayError("[mtoa] Failed to export scene to ass");
      }

      return status;
   }

   // Get render view panel
   MString renderViewPanelName = "";
   if (!batch)
   {
      MStringArray allPanelNames;
      MGlobal::executeCommand("getPanel -scriptType renderWindowPanel", allPanelNames);
      if (allPanelNames.length() > 0)
         renderViewPanelName = allPanelNames[0];
   }

   // Note: Maya seems to internally calls the preRender preLayerRender scripts
   //       as well as the postRender and postLayerRender ones

   CMayaScene::End(); // In case we're already rendering (e.g. IPR).

   // Check if in multiframe mode
   if (multiframe)
   {
      // TODO: This really needs to go. We're translating the whole scene for a couple of
      // render options.

      // If in batch rendering mode, check if a port was given to use for communication
      int port = batch && args.isFlagSet("port") ? args.flagArgumentInt("port", 0) : -1;

      double startframe;
      double endframe;
      double byframestep;

      if (renderGlobals.isAnimated())
      {
         startframe = renderGlobals.frameStart.as(MTime::uiUnit());
         endframe = renderGlobals.frameEnd.as(MTime::uiUnit());
         byframestep = renderGlobals.frameBy;
      }
      else
      {
         // FIXME which one should it be?
         // startframe = renderGlobals.frameStart.as(MTime::uiUnit());
         startframe = MAnimControl::currentTime().as(MTime::uiUnit());
         endframe = startframe;
         byframestep = 1;
      }

      // Find cameras to render
      MDagPathArray cameras;
      if (args.isFlagSet("camera"))
      {
         MSelectionList sel;
         args.getFlagArgument("camera", 0, sel);
         MDagPath dagPath;
         if (sel.getDagPath(0, dagPath) != MStatus::kSuccess)
         {
            AiMsgError("[mtoa] Could not get path to camera");
            return MStatus::kFailure;
         }
         cameras.append(dagPath);
      }
      else
      {
         MItDag dagIterCameras(MItDag::kDepthFirst, MFn::kCamera);
         // get all renderable cameras
         for (dagIterCameras.reset(); (!dagIterCameras.isDone()); dagIterCameras.next())
         {
            if (!dagIterCameras.getPath(dagPath))
            {
               AiMsgError("[mtoa] Could not get path for DAG iterator");
               return MStatus::kFailure;
            }

            MFnDependencyNode camDag(dagIterCameras.item());
            if (camDag.findPlug("renderable").asBool())
            {
               cameras.append(dagPath);
            }
         }
      }

      // Check if any camera should be saved to render view
      MString saveToRenderView = "";
      if (renderViewPanelName.length() > 0 && args.isFlagSet("saveToRenderView"))
      {
         saveToRenderView = args.flagArgumentString("saveToRenderView", 0);
      }

      std::set<double> frameSet;

      if (args.isFlagSet("seq"))
      {
         MString seq;
         args.getFlagArgument("seq", 0, seq);
         if (seq.length() > 0)
         {
            MStringArray seqArr;
            if (seq.index(';') == -1)
               seq.split(' ', seqArr);
            else
               seq.split(';', seqArr);
            for (unsigned int i = 0; i < seqArr.length(); ++i)
            {
               MString elem = seqArr[i];
               const int id = elem.indexW(MString(".."));
               if (id == -1) // just one frame
                  frameSet.insert(elem.asDouble());
               else if (id > 0)
               {
                  const int id2 = elem.index(':');
                  const double startFrame = elem.substring(0, id - 1).asDouble();
                  const double endFrame = elem.substring(id + 2, (id2 == -1) ? (elem.length() - 1) : (id2 - 1)).asDouble();
                  const double step = (id2 == -1) ? 1.0 : elem.substring(id2 + 1, elem.length() - 1).asDouble();
                  for (double frame = startFrame; frame <= endFrame; frame += step)
                     frameSet.insert(frame);
               }
            }
         }
      }

      if (frameSet.size() == 0)
      {
         for (double framerender = startframe; framerender <= endframe; framerender += byframestep)
            frameSet.insert(framerender);
      }

      size_t frameIndex = 1, frameCount = frameSet.size();
      // Get Layer display name
      MObject layerObj = MFnRenderLayer::currentLayer();
      MString layerDisplayName = MFnDependencyNode(layerObj).name();
      int scriptExists = 0;

      MGlobal::executeCommand("exists renderLayerDisplayName", scriptExists);
      if (scriptExists)
         MGlobal::executeCommand("renderLayerDisplayName " + layerDisplayName, layerDisplayName);

      for (std::set<double>::const_iterator frameIt = frameSet.begin(); frameIt != frameSet.end(); ++frameIt)
      {
         const double framerender = *frameIt;
         MGlobal::viewFrame(framerender);
         CMayaScene::ExecuteScript(renderGlobals.preRenderMel);

         // FIXME: do we really need to reset everything each time?
         CMayaScene::Begin(batch ? MTOA_SESSION_BATCH : MTOA_SESSION_SEQUENCE);

         CArnoldSession* arnoldSession = CMayaScene::GetArnoldSession();
         CRenderSession* renderSession = CMayaScene::GetRenderSession();
         arnoldSession->SetExportFrame(framerender);

         CMayaScene::Export(selectedPtr);

         if (renderViewPanelName.length() > 0)
            renderSession->SetRenderViewPanelName(renderViewPanelName);

         if (port != -1)
         {         
            AiNodeEntryInstall(AI_NODE_DRIVER, AI_TYPE_NONE,
                           "batch_progress_driver", "mtoa",
                           (AtNodeMethods*) batch_progress_driver_mtd, AI_VERSION);
         }

         for (unsigned int arrayIter = 0; (arrayIter < cameras.length()); arrayIter++)
         {
            const MDagPath& cameraDagPath = cameras[arrayIter];

            // It is ok to set the camera here, because if camera is no set at export time,
            // all the cameras are exported during the export.
            arnoldSession->SetExportCamera(cameraDagPath);
            renderSession->SetResolution(width, height);

            // append the batch progress driver at the end of the list if port flag has been added
            if (port != -1)
            {
               AtNode* options = AiUniverseGetOptions();
               AtArray* oldOutputs = AiNodeGetArray(options, "outputs");
               const unsigned oldCount = AiArrayGetNumElements(oldOutputs);
               AtArray* newOutputs = AiArrayAllocate(oldCount + 1, 1, AI_TYPE_STRING);
               for (unsigned i = 0; i < oldCount; ++i)
                  AiArraySetStr(newOutputs, i, AiArrayGetStr(oldOutputs, i));

               AtNode* filterNode = AiNode("box_filter");
               AiNodeSetStr(filterNode, "name", "progress_driver_filter");

               AtNode* progressDriver = AiNode("batch_progress_driver");
               AiNodeSetStr(progressDriver, "name", "progress_driver");
               AiNodeSetInt(progressDriver, "port", port);

               AiArraySetStr(newOutputs, oldCount, "Z FLOAT progress_driver_filter progress_driver");
               AiNodeSetArray(options, "outputs", newOutputs);
            }
            
            if (batch)
            {
               // Skipt file render if it already exists
               if(renderGlobals.skipExistingFrames)
               {
                  MStringArray imageFilenames = arnoldSession->GetActiveImageFilenames();
                  if(imageFilenames.length() > 0 && fileExists(imageFilenames[0].asChar()))
                  {
                        MGlobal::displayInfo("[mtoa] Skipping existing image: " + imageFilenames[0]);
                        continue;
                  }
               }

               int batchStatus = renderSession->DoBatchRender();
               if (batchStatus != AI_SUCCESS)
               {
                  CMayaScene::End();
                  MGlobal::displayError("[mtoa] Failed batch render");
                  if(port != -1 && batchStatus == AI_ABORT) {
                      MRenderUtil::sendRenderProgressInfo("", -111); // magic number for abort/kill
                  }
                  return MS::kFailure;
               }
            }
            else
            {
               MString msg = "[mtoa] Rendering Frame: ";
               msg +=  (int)framerender;
               msg +=  " (" ;
               msg += (int)frameIndex;
               msg += "/";
               msg += (int)frameCount;
               msg += ")  Camera: ";
               msg +=  cameraDagPath.partialPathName() ;
               msg += "  Layer: ";
               msg += layerDisplayName;

               MGlobal::displayInfo(msg);
               MStringArray imageFilenames = arnoldSession->GetActiveImageFilenames();
               
               // Skipt file render if it already exists
               if(renderGlobals.skipExistingFrames)
               {
                  if(imageFilenames.length() > 0 && fileExists(imageFilenames[0].asChar()))
                  {
                        MGlobal::displayInfo("[mtoa] Skipping existing image: " + imageFilenames[0]);
                        continue;
                  }
               }

               for (size_t i = 0; i < imageFilenames.length(); ++i)
                  MGlobal::displayInfo("\t" + imageFilenames[i]);

               int status = renderSession->DoInteractiveRender();
               if (status != AI_SUCCESS)
               {
                  CMayaScene::End();
                  if (status == AI_INTERRUPT)
                     MGlobal::displayInfo("[mtoa] Sequence render aborted");
                  else
                     MGlobal::displayError("[mtoa] Failed sequence render");
                  return MS::kFailure;
               }
               // Save the image to render view if requested
               if (saveToRenderView == "all" || saveToRenderView == MFnDependencyNode(cameraDagPath.transform()).name()) {
                  CMayaScene::ExecuteScript("renderWindowMenuCommand(\"keepImageInRenderView\", \"" + renderViewPanelName + "\")");
               }
            }
         }

         CMayaScene::End();
         CMayaScene::ExecuteScript(renderGlobals.postRenderMel);
         ++frameIndex;
      }
   }

   // or single frame interactive mode
   else
   {
      MSelectionList sel;
      args.getFlagArgument("camera", 0, sel);
      MDagPath camera;
      // FIXME: at scene open the animation bar in Maya maybe be off sync,
      // ie it shows 0 but currentTime -q returns 1. Render is correct as it's indeed
      // done for frame 1
      // MGlobal::viewFrame(currentFrame);

      CMayaScene::ExecuteScript(renderGlobals.preRenderMel);

      // CMayaScene::ExportAndRenderFrame(MTOA_SESSION_RENDER, selected);

      CMayaScene::Begin(MTOA_SESSION_RENDER);

      CArnoldSession* arnoldSession = CMayaScene::GetArnoldSession();
      CRenderSession* renderSession = CMayaScene::GetRenderSession();

      if (MStatus::kSuccess == sel.getDagPath(0, camera)) arnoldSession->SetExportCamera(camera);
      CMayaScene::Export(selectedPtr);
      renderSession->SetResolution(width, height);
      // Set the render session camera.
      renderSession->SetCamera(camera);
      // And render view panel
      if (renderViewPanelName.length() > 0)
         renderSession->SetRenderViewPanelName(renderViewPanelName);

      // Start the render.
      int stat = renderSession->DoInteractiveRender();
      if (stat != AI_SUCCESS)
         status = MS::kFailure;

      CMayaScene::End();
      CMayaScene::ExecuteScript(renderGlobals.postRenderMel, false, true);

      // Workaround for overriding the render view caption that Maya
      // sets after rendering is finished. Set the last caption again 
      // deferred to override Maya's caption.
      const MString& captionCmd = GetLastRenderViewCaptionCommand();
      if (captionCmd != "")
      {
         MGlobal::executeCommandOnIdle(captionCmd, false);
      }

      // DEBUG_MEMORY;
   }

   return status;
}
