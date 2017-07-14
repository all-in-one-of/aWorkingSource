# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

class Profile(object):

    class Constants:
        pass

    class DefaultConstants:
        pass

    def lightCreated(self, scriptItem):
        pass

    def skydomeCreated(self, scriptItem):
        pass

    def rigCreated(self, scriptItem):
        pass

    def masterMaterialCreated(self, scriptItem):
        pass

    def shaderSelected(self, scriptItem):
        pass

    def getConstant(self, name):
        return self.__class__.Constants.__dict__.get(name) or \
                self.__class__.DefaultConstants.__dict__.get(name)

    def getLightShaderType(self):
        raise RuntimeError('getLightShaderType() must be implemented on this profile')

    def getMaterialParameterName(self):
        raise RuntimeError('getMaterialParameterName() must be implemented on this profile')

    def getMaterialParameterValue(self):
        raise RuntimeError('getMaterialParameterValue() must be implemented on this profile')

    def shouldIncludeShader(self, name, location):
        return True

    def getRendererName(self):
        raise RuntimeError('getRendererName() must be implemented on this profile')



