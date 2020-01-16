# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


from LightBarndoor import *
from LightCoi import *
from LightConeAngle import *
from LightDecay import *
from LightDistFalloff import *
from LightQuad import *
from LightRadius import *
from LightRotateCoi import *
from LightSlideMap import *
from LightTranslateCoi import *
from MeasurementTool import *
from PlaceAlongNormal import *
from PlaceSpecular import *
from Rotate import *
from Scale import *
from Translate import *

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
