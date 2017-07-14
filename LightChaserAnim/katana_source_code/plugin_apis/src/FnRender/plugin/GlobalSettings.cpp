// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/GlobalSettings.h"

#include <string>
#include <iostream>
#include <fstream>
#include <map>

namespace Foundry
{
namespace Katana
{
namespace Render
{

GlobalSettings::GlobalSettings(FnScenegraphIterator rootIterator, const std::string& rendererNamespace) : _rootIterator(rootIterator)
{
    initialise(rendererNamespace);
}

int GlobalSettings::initialise(const std::string& rendererNamespace)
{
    _globalSettingsAttr = _rootIterator.getAttribute(rendererNamespace + "GlobalStatements");
    if(!_globalSettingsAttr.isValid())
        return 1;

    return 0;
}

}
}
}
