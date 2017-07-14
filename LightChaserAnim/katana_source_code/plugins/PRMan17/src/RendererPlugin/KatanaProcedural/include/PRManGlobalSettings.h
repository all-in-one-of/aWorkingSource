// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANGLOBALSETTINGS_H_
#define PRMANGLOBALSETTINGS_H_

#include <FnRender/plugin/GlobalSettings.h>

#include <ri.h>
#include <rx.h>

#include <string>
#include <map>

namespace FnKatRender = Foundry::Katana::Render;

class PRManGlobalSettings : public FnKatRender::GlobalSettings
{
    public:
       PRManGlobalSettings( FnKat::FnScenegraphIterator rootIterator,
                            std::string dsoPath = "",
                            std::string shaderPath = ""  ) : FnKatRender::GlobalSettings( rootIterator, "prman" ),
                                                             _dsoPath( dsoPath ),
                                                             _shaderPath( shaderPath ),
                                                             _displaySearchPathUsed( false ),
                                                             _proceduralSearchPathUsed( false ),
                                                             _shaderSearchPathUsed( false ) {}
       virtual ~PRManGlobalSettings() {}
       virtual int initialise();

       void applyGlobalSettings();
       int applyCameraGlobalSettings( FnAttribute::GroupAttribute cameraAttr );
       int applyOutputChannelsGlobalSettings( FnAttribute::GroupAttribute outputChannelsAttr );

       int recurseSubOptions( std::string optionsName, std::string& optionName, FnAttribute::GroupAttribute subOptions, void (*riCommand)(char *, ...) );
       int applyOption( std::string name, std::string parameterName, std::string parameterType, FnKat::Attribute parameterAttr, void (*riCommand)(char *, ...) );
       std::string getOptionParameter( std::string name, std::string type, int size );

       void getGlobalTypeMap( std::map<std::string,std::string> &prmanGlobalsMap );

       std::string getFrameBeginCameraPath();
       std::string getShaderPath() { return _shaderPath; }

    private:
       std::string _dsoPath;
       std::string _shaderPath;
       bool _displaySearchPathUsed;
       bool _proceduralSearchPathUsed;
       bool _shaderSearchPathUsed;
};

#endif /* PRMANGLOBALSETTINGS_H_ */
