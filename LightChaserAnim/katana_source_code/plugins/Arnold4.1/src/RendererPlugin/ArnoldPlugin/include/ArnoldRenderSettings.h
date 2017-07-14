// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLDRENDERSETTINGS_H_
#define ARNOLDRENDERSETTINGS_H_

#include <FnRender/plugin/RenderSettings.h>

namespace FnKatRender = Foundry::Katana::Render;

class ArnoldRenderSettings : public FnKatRender::RenderSettings
{
    public:
        ArnoldRenderSettings( FnKat::FnScenegraphIterator rootIterator ) : FnKatRender::RenderSettings( rootIterator ) { initialise(); }
        virtual ~ArnoldRenderSettings() {}
        virtual int initialise();

        std::string getCropWindowAsString() const;
};

#endif /* ARNOLDRENDERSETTINGS_H_ */
