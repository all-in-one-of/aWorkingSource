# Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
"""
Python package implementing a Example Viewer tab with simple drawing of mesh
geometry data as points. This tab uses a C++ library to perform the actual
drawing.
"""

# Append the Example viewer library to the path to allow it to be imported
import os, sys
import ManipulatorMenu

# Scan KATANA_RESOURCES directories for the one that contains 'ExampleViewer.so'
# and then add that to the system path, to allow us to import it below.

resources = os.getenv('KATANA_RESOURCES', '')
resources = resources.split(os.pathsep) if resources != '' else []
libExtension = "pyd" if sys.platform == "win32" else "so"

for resourcePath in resources:
    if not resourcePath:
        continue
    libPath = os.path.abspath(os.path.join(resourcePath, 'Libs'))
    if(os.path.isfile(os.path.join(libPath, 'ExampleViewer.%s' % libExtension))
            or os.path.isfile(os.path.join(libPath, 'ExampleViewer_d.%s' % libExtension))):
        sys.path.append(libPath)


from ExampleViewerTab import ExampleViewerTab

PluginRegistry = [
    ("KatanaPanel", 2.0, "Example Viewer", ExampleViewerTab),
]
