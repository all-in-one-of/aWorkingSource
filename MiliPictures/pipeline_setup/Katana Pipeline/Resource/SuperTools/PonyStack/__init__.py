# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

import Katana
import v1 as PonyStack

if PonyStack:
    PluginRegistry = [
        ("SuperTool", 2, "PonyStack",
                (PonyStack.PonyStackNode,
                        PonyStack.GetEditor)),
    ]
