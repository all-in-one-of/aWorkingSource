# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

PluginRegistry=[]

import ScenegraphXmlAsset
PluginRegistry.append(
    ("ImportomaticModule", "1.0.0",  "ScenegraphXmlAsset", (ScenegraphXmlAsset.Register, ScenegraphXmlAsset.RegisterGUI)),
)
