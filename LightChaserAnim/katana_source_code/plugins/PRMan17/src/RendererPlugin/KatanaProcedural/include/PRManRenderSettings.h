// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANRENDERSETTINGS_H_
#define PRMANRENDERSETTINGS_H_

#include <FnRender/plugin/RenderSettings.h>

namespace FnKatRender = Foundry::Katana::Render;

class PRManRenderSettings : public FnKatRender::RenderSettings
{
    public:
        PRManRenderSettings( FnKat::FnScenegraphIterator rootIterator ) : FnKatRender::RenderSettings( rootIterator ) { initialise(); }
        virtual ~PRManRenderSettings() {}
        virtual int initialise();

        std::string getDataWindowWidthAsString() const;
        std::string getDataWindowHeightAsString() const;
        std::string getShutterAsString() const;
        std::string getCropWindowAsString() const;
};

#endif /* PRMANRENDERSETTINGS_H_ */
