# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


class BaseProxyLoader(object):
    '''
    Base class for a geometry proxy plugin. This class will
    be used by the viewer for the locations for which a
    "proxies.viewer" attribute exist. This attribute will
    have a file path to a proxy geometry file.
    '''

    @classmethod
    def getFileExtensions(cls):
        '''
        This function must be implemented by the plugin.
        It must return a list with file extensions that
        will trigger the plugin.
        '''
        raise NotImplementedError

    @classmethod
    def createProxyAttr(cls, proxyFile, time, args):
        '''
        This function must be implemented by the plugin.
        Must return a GroupAttribute that sets up an Op
        and its arguments that will load the specified proxy file.

        @type proxyFile: C{str}
        @type time: C{int}
        @type args: C{FnAttribute.GroupAttribute}
        @rtype: C{FnAttribute.GroupAttribute}
        @param proxyFile: The file specified in the "proxies.viewer" attr.
        @param time: The current time of the Katana scene.
        @param args: The GroupAttribute specified by "proxies.args" where some
            extra arguments can be passed to construct the Op.
        @return: A GroupAttribute that sets up an Op and its arguments that
            will load the specified proxy file. The GroupAttribute should
            contain a 'opType' StringAttribute specifying the Op as well
            as a GroupAttribute named 'opArgs' with the arguments to pass
            to the Op.
        '''
        raise NotImplementedError
