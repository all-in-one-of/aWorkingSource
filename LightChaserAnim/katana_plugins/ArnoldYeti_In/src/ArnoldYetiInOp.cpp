// Description: An Op that creates a renderer procedural location for the Yeti Arnold procedural plugin DSO
// Author: mike
// Date: 2015-06-27

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>


namespace
{


class ArnoldYetiInOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface& interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface& interface)
    {
        // User facing attributes

        interface.setAttr("type", FnAttribute::StringAttribute("renderer procedural"));

        // Load the plugin by path...
        interface.setAttr("rendererProcedural.procedural", FnAttribute::StringAttribute("pgYetiArnold"));
        // ...but use the node directly once it's loaded
        interface.setAttr("rendererProcedural.node", FnAttribute::StringAttribute("pgYetiArnold"));
        interface.setAttr("rendererProcedural.includeCameraInfo", FnAttribute::StringAttribute("None"));
        // classic, scenegraphAttr, typedArguments; default to typedArguments
        interface.setAttr("rendererProcedural.args.__outputStyle", FnAttribute::StringAttribute("typedArguments"));
        // Skip builtin parameters from Katana; the plugin doesn't use them
        interface.setAttr("rendererProcedural.args.__skipBuiltins", FnAttribute::IntAttribute(1));

        // Get shutter / MB fallback information, in case samples is empty/missing
        FnAttribute::FloatAttribute shutterOpenAttr = interface.getOpArg("system.timeSlice.shutterOpen");
        FnAttribute::FloatAttribute shutterCloseAttr = interface.getOpArg("system.timeSlice.shutterClose");
        FnAttribute::IntAttribute numSamplesAttr = interface.getOpArg("system.timeSlice.numSamples");

        FnAttribute::StringAttribute filenameAttr = interface.getOpArg("filename");
        FnAttribute::StringAttribute proxyAttr = interface.getOpArg("proxy");
        FnAttribute::IntAttribute disableBouningBoxAttr = interface.getOpArg("disableBoundingbox");

        double bounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        std::string filename = filenameAttr.getValue("", false);
        std::string yetiCacheInfoExe = "pgYetiCacheInfo";
#ifdef _WIN32
        yetiCacheInfoExe += ".exe";
        char *outputFilename = ::_tempnam("C:\\Windows\\TEMP", "ktoa-arnoldyetiop-");
        bool success = outputFilename != NULL;
#else
        char outputFilename[] = "/tmp/ktoa-arnoldyetiop-XXXXXX";
        int tmpFD = ::mkstemp(outputFilename);
        bool success = tmpFD > -1;
        if (success)
            ::close(tmpFD); // but don't unlink!  Let the subprocess open and write it.
#endif
        if (!bool(disableBouningBoxAttr.getValue()))
        {


            if (success)
            {
                std::string command = yetiCacheInfoExe + " --bounding_box --file " + filename + " > " + outputFilename;
                if (::system(NULL))
                {
                    int result = ::system(command.c_str());
                    if (result == 0)
                    {
                        std::ifstream boundsIn(outputFilename);
                        if (boundsIn.good())
                        {
                            bool gotBounds = true;
                            char c;
                            boundsIn >> c;
                            if (c != '(')
                                gotBounds = false;
                            boundsIn >> bounds[0];
                            boundsIn >> bounds[2];
                            boundsIn >> bounds[4];
                            boundsIn >> c;
                            if (c != ')')
                                gotBounds = false;
                            boundsIn >> c;
                            if (c == ' ')
                                boundsIn >> c;
                            if (c != '(')
                                gotBounds = false;
                            boundsIn >> bounds[1];
                            boundsIn >> bounds[3];
                            boundsIn >> bounds[5];
                            boundsIn >> c;
                            if (c != ')')
                                gotBounds = false;
                            if (gotBounds)
                            {
                                FnAttribute::DoubleAttribute boundAttr(&bounds[0], 6, 1);
                                interface.setAttr("bound", boundAttr);
                            }
                        }
                        boundsIn.close();
                    }
                    else
                    {
                        Foundry::Katana::ReportWarning(interface, "[KtoA] Could not get bounds for file: " + filename);
                    }
                }
                else
                {
                    Foundry::Katana::ReportWarning(interface, "[KtoA] Could not get command processor to find bounds for file: " + filename);
                }
#ifdef WIN32
                ::_unlink(outputFilename);
                ::free(outputFilename);
#else
                ::unlink(outputFilename);
#endif
            }
            else
            {
                Foundry::Katana::ReportWarning(interface, "[KtoA] Could not generate temp file to get bound for file: " + filename);
            }



        }

        // The following parameters are built-in the Yeti node (and won't end up declared)

        interface.setAttr("rendererProcedural.args.filename", filenameAttr );

        // set proxy alembic file
        if ( !proxyAttr.getValue().empty() )
        {
            interface.setAttr("proxies.viewer.Proxy_Op0.opType", FnAttribute::StringAttribute("AlembicIn"));
            interface.setAttr("proxies.viewer.Proxy_Op0.opArgs.fileName", FnAttribute::StringAttribute(proxyAttr));            
        }

        FnAttribute::StringAttribute imageSearchPathAttr = interface.getOpArg("imageSearchPath");
        interface.setAttr("rendererProcedural.args.imageSearchPath", imageSearchPathAttr );
        FnAttribute::FloatAttribute samplesAttr = interface.getOpArg("samples");
        float shutterOpen  = shutterOpenAttr.getValue(0.0f, false);  //FnKat::GetShutterOpen(interface);
        float shutterClose = shutterCloseAttr.getValue(0.0f, false); //FnKat::GetShutterClose(interface);
        float motionStart  =  std::numeric_limits<float>::max();
        float motionEnd    = -std::numeric_limits<float>::max();
        if (samplesAttr.isValid() && samplesAttr.getNumberOfValues() > 0)
        {
            interface.setAttr("rendererProcedural.args.samples", samplesAttr);
            FnAttribute::FloatConstVector samples = samplesAttr.getNearestSample(0.0f);
            for (size_t i = 0; i < samples.size(); ++i)
            {
                motionStart = std::min(motionStart, samples[i]);
                motionEnd   = std::max(motionEnd,   samples[i]);
            }
        }
        else
        {
            int numSamples = numSamplesAttr.getValue(1, false); //FnKat::GetNumSamples(interface);
            std::vector<float> relativeSamples;
            if (numSamples > 1)
            {
                relativeSamples.reserve(numSamples);
                for (int i = 0; i < numSamples; ++i)
                {
                    relativeSamples.push_back(shutterOpen + i * (shutterClose - shutterOpen) / (numSamples - 1));
                    motionStart = std::min(motionStart, relativeSamples.back());
                    motionEnd   = std::max(motionEnd,   relativeSamples.back());
                }
            }
            else
            {
                relativeSamples.push_back(0.0f);
                motionStart = motionEnd = 0.0f;
            }
            FnAttribute::FloatAttribute relativeSamplesAttr(&relativeSamples[0], relativeSamples.size(), 1);
            interface.setAttr("rendererProcedural.args.samples", relativeSamplesAttr);
        }
        if (shutterOpen < shutterClose)
        {
            motionStart = std::min(motionStart, shutterOpen);
            motionEnd   = std::max(motionEnd,   shutterClose);
        }
        interface.setAttr("rendererProcedural.args.motion_start", FnAttribute::FloatAttribute(motionStart));
        interface.setAttr("rendererProcedural.args.motion_end",   FnAttribute::FloatAttribute(motionEnd));
        interface.setAttr("rendererProcedural.args.frameRelativeSamples", FnAttribute::IntAttribute(1));

        FnAttribute::IntAttribute frameAttr = interface.getOpArg("frame");
        interface.setAttr("rendererProcedural.args._frame", frameAttr); // Special name, see hints below
        FnAttribute::FloatAttribute densityAttr = interface.getOpArg("density");
        interface.setAttr("rendererProcedural.args.density", densityAttr );
        FnAttribute::FloatAttribute lengthAttr = interface.getOpArg("length");
        interface.setAttr("rendererProcedural.args.length", lengthAttr);
        FnAttribute::FloatAttribute widthAttr = interface.getOpArg("width");
        interface.setAttr("rendererProcedural.args.width", widthAttr);
        FnAttribute::IntAttribute verbosityAttr = interface.getOpArg("verbose");
        // Support both old-style 'verbose' and the new 'verbosity' params
        interface.setAttr("rendererProcedural.args.verbose", verbosityAttr);
        interface.setAttr("rendererProcedural.args.verbosity", verbosityAttr);

        // The following parameters are honored by the Yeti node, but have to be declared

        FnAttribute::IntAttribute threadsAttr = interface.getOpArg("threads");
        if (threadsAttr.getValue() != 0)
            interface.setAttr("rendererProcedural.args.threads", threadsAttr);
        FnAttribute::FloatAttribute minPixelWidthAttr = interface.getOpArg("min_pixel_width");
        interface.setAttr("rendererProcedural.args.min_pixel_width", minPixelWidthAttr);
        FnAttribute::IntAttribute modeAttr = interface.getOpArg("mode");
        interface.setAttr("rendererProcedural.args.mode", modeAttr);

        // Allow interactive transform manipulation in Katana

        FnAttribute::StringAttribute makeInteractiveAttr = interface.getOpArg("makeInteractive");
        if (makeInteractiveAttr.isValid() && std::string(makeInteractiveAttr.getValue("Yes", false)) == "Yes")
        {
            interface.setAttr("xform.interactive", interface.getOpArg("xform"));
            interface.setAttr("attributeEditor.exclusiveTo", interface.getOpArg("exclusiveTo"));
        }
        else
            interface.setAttr("xform.transform0", interface.getOpArg("xform"));

        // Special handling for samples, frame, and frameRelativeSamples parameters

        // These extra params are interpreted by the procedural args translator
        // to set the DSO argument types and declarations correctly
        std::vector<std::string> arrayHint;
        // The samples array needs to be an array of keys (of one value each) due to a Yeti quirk
        arrayHint.push_back("key_array");
        arrayHint.push_back("true");
        FnAttribute::StringAttribute arrayHintAttr(arrayHint);
        interface.setAttr("rendererProcedural.args.arnold_hint__samples", arrayHintAttr);

        std::vector<std::string> frameHint;
        // The name 'frame' conflicts with a Katana builtin arg, so we skip those
        // and rename '_frame' to 'frame' afterwards
        frameHint.push_back("rename");
        frameHint.push_back("frame");
        FnAttribute::StringAttribute frameHintAttr(frameHint);
        interface.setAttr("rendererProcedural.args.arnold_hint___frame", frameHintAttr);

        std::vector<std::string> frameRelativeSamplesHint;
        // frameRelativeSamples needs to be an Arnold bool type on the procedural
        frameRelativeSamplesHint.push_back("type");
        frameRelativeSamplesHint.push_back("boolean");
        FnAttribute::StringAttribute frameRelativeSamplesHintAttr(frameRelativeSamplesHint);
        interface.setAttr("rendererProcedural.args.arnold_hint__frameRelativeSamples", frameRelativeSamplesHintAttr);
    }
};


DEFINE_GEOLIBOP_PLUGIN(ArnoldYetiInOp)


} // anonymous


void registerPlugins()
{
    REGISTER_PLUGIN(ArnoldYetiInOp, "ArnoldYeti_In", 0, 1);
}
