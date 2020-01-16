import sys
import os
# add parent directory as syspath.  
pa = os.path.abspath(os.path.join(os.path.dirname(__file__),os.pardir))
sys.path.append(pa)

import rpyc

#make connection:
client = rpyc.classic.connect("localhost",52077)
client._config['timeout'] = 15

#get remote modules
falcon_engine = client.modules.falcon_engine
unreal_engine = client.modules.unreal_engine

ClassModule = client.modules["unreal_engine.classes"]
StructsModule = client.modules["unreal_engine.structs"]
EnumsModule = client.modules["unreal_engine.enums"]

#Classes
FalconObject = ClassModule.FalconObject
Object = ClassModule.Object
StrProperty = ClassModule.StrProperty
IntProperty = ClassModule.IntProperty
Blueprint = ClassModule.Blueprint
K2Node_DynamicCast = ClassModule.K2Node_DynamicCast

#Structs
EdGraphPinType = StructsModule.EdGraphPinType
EdGraphTerminalType = StructsModule.EdGraphTerminalType
Vector = StructsModule.Vector
Rotator = StructsModule.Rotator
EPinContainerType = EnumsModule.EPinContainerType

#Flags
CLASS_CONFIG = unreal_engine.CLASS_CONFIG
CLASS_DEFAULT_CONFIG = unreal_engine.CLASS_DEFAULT_CONFIG
CPF_CONFIG = unreal_engine.CPF_CONFIG
