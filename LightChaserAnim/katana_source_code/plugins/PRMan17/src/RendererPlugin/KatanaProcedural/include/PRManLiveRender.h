// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANLIVERENDER_H_
#define PRMANLIVERENDER_H_

#include <PRManPluginState.h>
#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <FnAttribute/FnAttribute.h>

#include <map>
#include <queue>
#include <deque>


class PRManLiveRender
{
    public:
        PRManLiveRender( bool raytrace, PRManPluginState* sharedState );
        virtual ~PRManLiveRender() {}

        enum RerenderIllumination
        {
            LIGHT_ON,
            LIGHT_OFF,
            TURN_LIGHT_ON,
            TURN_LIGHT_OFF
        };

        enum LightEnabledState
        {
            LIGHT_ENABLE_OFF,
            LIGHT_ENABLE_ON,
            LIGHT_ENABLE_UNDEFINED,
        };

        struct Update
        {
            std::string type;
            std::string location;
            FnAttribute::GroupAttribute attributesAttr;
            FnAttribute::GroupAttribute xformAttr;
        };
        typedef std::queue<Update> UpdateQueue;
        typedef std::deque<Update> UpdateDeque;
        typedef std::map<std::string, bool> LocationEnableMap;

        void processUpdates( FnAttribute::GroupAttribute updateAttribute );
        void applyUpdates( FnKat::FnScenegraphIterator rootIterator );
        int getNumberOfUpdates();

    private:
        bool raytrace;
        PRManPluginState* sharedState;

        UpdateQueue cameras;
        UpdateQueue lights;
        UpdateQueue materials;
        UpdateQueue coordinateSystems;
        UpdateDeque lightLinks;
        std::map<std::string, UpdateDeque> deferredLightLinks;

        std::map<std::string, int> lightSwitches;
        std::map<std::string, LocationEnableMap> lightLinkOff;

        // Re-render stream marker
        int flushMarker;
        int prmanRerenderCount;

        std::string renderCamera;
        std::string lightsEdited;
        std::map<std::string, FnAttribute::GroupAttribute> lightShaderCache;

        std::string intToString( int number );
        std::string getStreamMarker( std::string markerId );
        std::string getNextStreamMarker();

        int updateCamera( FnKat::FnScenegraphIterator rootIterator );
        int updateLight( FnKat::FnScenegraphIterator rootIterator );
        int updateMaterial( FnKat::FnScenegraphIterator rootIterator );
        int updateCoordinateSystem();
        int updateLightLink( FnKat::FnScenegraphIterator rootIterator );

        // Determine whether the passed light link update should be queued
        bool queueLightLinkUpdate( Update& update  );

        int updateIlluminate( const std::string& lightLocation, bool on,  const std::string&  scopeName=""  );
        void updateCoshaders( FnAttribute::GroupAttribute materialAttr, FnAttribute::GroupAttribute infoAttr );
};

#endif /* PRMANLIVERENDER_H_ */
