// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLDRENDER_H_
#define ARNOLDRENDER_H_

#include <map>
#include <FnRender/plugin/RenderBase.h>
#include "change_q.h"

#ifndef _WIN32
#include <sys/times.h>
#include <sys/wait.h>
#include <getopt.h>
#else
#include "windowshelpers.h"
#endif

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sstream>
#include <memory>
#include <set>
#include <algorithm>
#include <queue>

#include <ArnoldRenderSettings.h>
#include <FnRender/plugin/CameraSettings.h>
#include <FnRender/plugin/GlobalSettings.h>
#include <ArnoldPluginState.h>

namespace FnKatRender = FnKat::Render;

// Function defined in driver_katana.cpp
extern void loadDriverKatana();


class ArnoldRender : public Foundry::Katana::Render::RenderBase
{
    public:
        ArnoldRender( FnKat::FnScenegraphIterator rootIterator, FnAttribute::GroupAttribute arguments );
        virtual int start();
        virtual int pause();
        virtual int stop();
        virtual int processControlCommand(const std::string& command);
        virtual int queueDataUpdates( FnAttribute::GroupAttribute updateAttribute );
        virtual int applyPendingDataUpdates();
        virtual bool hasPendingDataUpdates() const;

        virtual void configureDiskRenderOutputProcess(
                FnKatRender::DiskRenderOutputProcess& diskRenderOutputProcess,
                const std::string& outputName,
                const std::string& outputPath,
                const std::string& renderMethodName,
                const float& frameTime ) const;

        std::string GetCheckpointFileName( const std::string& outputPath, const std::string& outputFileName );

        static Foundry::Katana::Render::RenderBase* create( FnKat::FnScenegraphIterator rootIterator, FnAttribute::GroupAttribute args )
        {
            return new ArnoldRender( rootIterator, args );
        }
        static void flush() {};

    private:
        std::string renderMethodName;
        std::string renderSettingsCamera;
        std::string activeLiveRenderCamera;
        long renderResult;
        std::string local_output_address;
        std::string local_output_channel;
        std::vector<std::string> local_output_channels;
        std::string local_output_info_file;
        std::string local_render_finished_filename;

        std::string local_filter_filename;
        long local_output_width;
        long local_output_height;
        int local_overscan[4];
        int local_frame_origin[2];
        int local_display_window_size[2];
        std::string local_dumpfile_filename;
        bool local_expand_procedurals_for_ass_dump;
        std::string local_crop_window_default;
        std::string local_crop_window;
        std::string original_crop_window;

        int local_frame_id;
        std::vector<int> local_frame_ids;
        std::string local_frame_name;
        int local_force_expand;
        int local_interactive_render;
        int local_batch_render;
        int local_generate_ids;

        bool partialUpdate;

        FnKat::FnScenegraphIterator local_root_iterator;
        bool local_any_checkpoint_file;

        int AA_samples;
        int AA_samples_prepass;
        int AA_samples_prepass_rerender;
        int AA_samples_step;

        bool skip_reset_options;

        void SetArguments( ArnoldRenderSettings &renderSettings );
        void SetupRender( FnKat::FnScenegraphIterator rootIterator, ArnoldRenderSettings &renderSettings );
        void InitArnold( FnKat::FnScenegraphIterator rootIterator, ArnoldRenderSettings &renderSettings);
        void ProcessRerender( FnKat::FnScenegraphIterator rootIterator );
        AtNode* SetupDisplayDriver( const std::string& driverName, const std::string& frameName, const bool bucketCorners );

    private:
        ArnoldPluginState sharedState;

        struct TilePoint
        {
            int x;
            int y;

            TilePoint() : x(-1), y(-1) {}
            TilePoint(int x_, int y_) : x(x_), y(y_) {}

            // This determines the iteration order of the set (which
            //  determines the bucket list order passed to Arnold).
            bool operator < (const TilePoint &other) const
            {
                if (y != other.y) return y < other.y;
                return x < other.x;
            }
        };
        typedef std::set<TilePoint> TilePointSet;

        struct OutputInfo
        {
            std::string file;
            std::string channel;
            std::set<std::string> isolateLights;
            std::string cameraName;
            bool lightAgnostic;

            // filled in by initArnold()
            std::string outputString;

            // checkpoint info
            std::string checkpointFile;
            TilePointSet validTilePoints;

            std::vector<std::string> lightExpressions;

            typedef std::pair<std::string, FnAttribute::GroupAttribute> PreprocessDsoEntry;
            typedef std::vector<PreprocessDsoEntry> PreprocessDsoEntryList;

            PreprocessDsoEntryList preprocessEntries;
            PreprocessDsoEntryList postprocessEntries;

            OutputInfo() : lightAgnostic(false) {}
            OutputInfo(const std::string & inFile,
                       const std::string & inChannel,
                       const std::set<std::string> & inIsolateLights,
                       const std::string & inCameraName,
                       bool inLightAgnostic) :
                        file(inFile), channel(inChannel),
                        isolateLights(inIsolateLights),
                        cameraName(inCameraName),
                        lightAgnostic(inLightAgnostic){}
        };

        typedef std::map<std::string, OutputInfo> OutputInfoMap;
        typedef std::vector<std::string> OutputInfoList;

        OutputInfoMap local_output_info_map;
        OutputInfoList local_output_info_list;

        const OutputInfo & _getOutputInfoByName(const std::string & name)
        {
            OutputInfoMap::const_iterator iter = local_output_info_map.find(name);
            if (iter == local_output_info_map.end())
            {
                AiMsgError("[kat] FATAL: unable to find output info by name (%s).", name.c_str());
                exit(1);
            }
            return (*iter).second;
        }

        struct RenderPassKey
        {
            std::string cameraName;
            std::set<std::string> isolateLights;

            RenderPassKey(const std::string &inCameraName,
                          const std::set<std::string> &inIsolateLights) :
                cameraName(inCameraName), isolateLights(inIsolateLights) {}

            bool operator < (const RenderPassKey &other) const
            {
                if (cameraName < other.cameraName) return true;
                if (cameraName > other.cameraName) return false;
                return (isolateLights < other.isolateLights);
            }
        };

        typedef std::map<RenderPassKey, std::vector<std::string> > RenderPassOutputMap;
        typedef std::map<std::string, std::vector<std::string> > StringVectorMap;

        // Map camera name and set of isolate light paths to a vector of output strings.
        // Each unique combination of cameraName/isolateLights must be rendered in a
        // separate Arnold pass.
        RenderPassOutputMap local_render_passes;

        // Maps camera path to a vector of output strings that don't care about which
        // lights are active.  Attach these outputs to the first render pass using the
        // camera, regardless of isolated lights.
        StringVectorMap local_light_agnostic_passes;

        struct CheckpointData
        {
            int tileSize[2];
            int numTiles[2];

            TilePointSet allTilePoints;

            CheckpointData()
            {
                tileSize[0] = tileSize[1] = -1;
                numTiles[0] = numTiles[1] = -1;
            }
        };
        CheckpointData local_checkpoint_data;
        std::vector<pid_t> local_checkpoint_watch_pids;

        // Constants for options which have no single-character code.
        enum option_constants
        {
            OUTPUT_INFO = 1,
            OUTPUT_ADDRESS,
            OUTPUT_CHANNEL,
            DUMP_ASSFILE,
            FRAME_ID,
            FRAME_TIME,
            FRAME_NAME,
            FRAME_WIDTH,
            FRAME_HEIGHT,
            PRODUCER_TEXT,
            CROP_WINDOW,
            BATCH_RENDER,
            SCREEN_WINDOW,
            FRAME_ORIGIN,
            DISPLAY_WINDOW_SIZE,
        };

        pid_t _forkCommand( const std::vector<std::string> & command )
        {
#ifdef _WIN32
            std::cerr << "Fork not implemented for WIN32 platform!";
            assert(false); // NOT IMPLEMENTED!!
            throw std::exception("Not implemented!");
#else
            pid_t childpid = ::fork();

            if (childpid < 0)
            {
                AiMsgInfo("[kat] Warning: unable to create checkpoint_watch"
                          " child process (process fork failed).");
                return childpid;
            }

            if (childpid == 0)
            {
                std::cerr << "Running checkpoint_watch command: ";

                std::vector<std::string>::const_iterator iter;
                // Convert the args to a const char* array.
                std::vector<const char *> charArgs;
                for (iter = command.begin(); iter != command.end(); ++iter)
                {
                    std::cerr << (*iter) << " ";
                    charArgs.push_back((*iter).c_str());
                }
                std::cerr << std::endl;
                charArgs.push_back(0);

                // restore stdout/stderr
                const char *origStdoutStr = ::getenv("KATANA_ORIG_STDOUT");
                const char *origStderrStr = ::getenv("KATANA_ORIG_STDERR");
                if (origStdoutStr != 0x0)
                {
                    int origStdout = ::atoi(origStdoutStr);
                    ::dup2(origStdout, STDOUT_FILENO);
                }
                if (origStderrStr != 0x0)
                {
                    int origStderr = ::atoi(origStderrStr);
                    ::dup2(origStderr, STDERR_FILENO);
                }

                ::execvp(charArgs[0], (char *const *)&charArgs[0]);

                _exit(1); // if we reach this, execvp failed: _exit child process with error
            }

            return childpid;
#endif
        }

        void PrintRenderResults( long renderResult )
        {
            std::string preamble = "[kat]  AiRender() returned ";
            switch (renderResult)
            {
                case AI_SUCCESS:
                    std::cerr << preamble << "AI_SUCCESS" << std::endl;
                    break;
                case AI_ABORT:
                    std::cerr << preamble << "AI_ABORT" << std::endl;
                    break;
                case AI_ERROR_WRONG_OUTPUT:
                    std::cerr << preamble << "AI_ERROR_WRONG_OUTPUT" << std::endl;
                    break;
                case AI_ERROR_NO_CAMERA:
                    std::cerr << preamble << "AI_ERROR_NO_CAMERA" << std::endl;
                    break;
                case AI_ERROR_BAD_CAMERA:
                    std::cerr << preamble << "AI_ERROR_BAD_CAMERA" << std::endl;
                    break;
                case AI_ERROR_VALIDATION:
                    std::cerr << preamble << "AI_ERROR_VALIDATION" << std::endl;
                    break;
                case AI_ERROR_RENDER_REGION:
                    std::cerr << preamble << "AI_ERROR_RENDER_REGION" << std::endl;
                    break;
                case AI_ERROR_OUTPUT_EXISTS:
                    std::cerr << preamble << "AI_ERROR_OUTPUT_EXISTS" << std::endl;
                    break;
                case AI_ERROR_OPENING_FILE:
                    std::cerr << preamble << "AI_ERROR_OPENING_FILE" << std::endl;
                    break;
                case AI_INTERRUPT:
                    std::cerr << preamble << "AI_INTERRUPT" << std::endl;
                    break;
                case AI_ERROR_UNRENDERABLE_SCENEGRAPH:
                    std::cerr << preamble << "AI_ERROR_UNRENDERABLE_SCENEGRAPH" << std::endl;
                    break;
                case AI_ERROR_NO_OUTPUTS:
                    std::cerr << preamble << "AI_ERROR_NO_OUTPUTS" << std::endl;
                    break;
                case AI_ERROR: // not actually possible for long to be AI_ERROR (-1)
                    std::cerr << preamble << "AI_ERROR" << std::endl;
                    break;
                default:
                    std::cerr << preamble << "unknown error: " << renderResult << std::endl;
                    break;
            }
        }

        typedef std::map<std::string, bool> LightLinkMap;
        typedef std::map<std::string, LightLinkMap> LocationLightLinkMap;
        LocationLightLinkMap deferredLightLinks;

        FnAttribute::GroupAttribute globalSettings;
        FnAttribute::GroupAttribute liveRenderSettingOverrides;

    private:
        //void loadOutputInfoFile( FnScenegraphIterator rootIterator );
        void processOutputs( FnKat::FnScenegraphIterator rootIterator, ArnoldRenderSettings &renderSettings );
        void adjustCheckpointDataForCropWindow();
        void doLaunchKatanaListener( FnKat::FnScenegraphIterator rootIterator );
        bool renderPassSetup( const std::string &cameraName,
                              const std::vector<std::string> &outputNames,
                              bool flushCache,
                              const std::string &origBucketScan,
                              int passCounter, int passTotal);
        void killCheckpointWatchProcesses();
        int getAssWriteIncludeOptions( FnAttribute::GroupAttribute ags );
        void runPassProcessPlugin( const OutputInfo::PreprocessDsoEntryList & entries, const char * entryPointName );
        void resetCropWindow( int* crop );
        void getLightExpressionsForBucketRender( FnKat::FnScenegraphIterator rootIterator, std::vector<std::string> & output );

        void queueLightLinkUpdates(LocationLightLinkMap &lightLinks, const std::string &lightFilter="");

        int RerenderXform( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderCamera( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderLights( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderLightsXform( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderMaterials( std::string& itemName, FnAttribute::GroupAttribute attributesAttr, FnAttribute::StringAttribute parentLocationAttr );
        int RerenderGlobals( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderRenderSettings( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderLiveRenderSettings( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderShadingOverrides( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
        int RerenderLightLink( std::string& itemName, FnAttribute::GroupAttribute attributesAttr );
};

#endif /* ARNOLDRENDER_H_ */
