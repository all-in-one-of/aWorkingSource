# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


import os, os.path, sys, re
import AssetAPI

_seqre = re.compile(r'(.*\.)(\(\s*(\d+)\s*-\s*(\d+)\s*\))?%(\d+)d(\..*)')
_filere = re.compile(r'(.*\.)(\d+)(\..*)')

_seqfmt = '%s%%0%dd%s'

'''
This simlpe File Sequence Plugin uses a Nuke-like file sequence format. For a
4 zero-padded file sequence the format would be, for example:
  image.%04d.exr
'''


class MockFileSequencePlugin(AssetAPI.BaseFileSequencePlugin):
    '''
    The file sequence plugin class.
    '''
    def getFileSequence(self, seqstr):
        return MockFileSequence(seqstr)

    def isFileSequence(self, seqstr):
        return bool(_seqre.match(seqstr))

    def buildFileSequenceString(self, prefix, suffix, padding):
        return _seqfmt % (prefix, padding, suffix)

    def findSequence(self, fileList):
        seq = set()
        nonseq = []
        for x in fileList:
            m = _filere.match(x)
            if m:
                g = m.groups()
                seq.add( (g[0], g[2], len(g[1])) )
            else:
                nonseq.append(x)
        seq = [MockFileSequence(self.buildFileSequenceString(x[0], x[1], x[2])) for x in seq]
        return seq, nonseq


class MockFileSequence(object):
    '''
    The class that implements the File Sequence.
    '''
    def __init__(self, seqstr):
        self.__seqstr = seqstr
        seqm = _seqre.match(self.__seqstr)
        if not seqm:
            raise ValueError, 'Not a valid sequence'
        g = seqm.groups()
        self.__prefix = g[0]
        self.__padding = int(g[4])
        self.__suffix = g[5]

        firstFrame = g[2]
        lastFrame = g[3]
        if firstFrame is not None and lastFrame is not None:
            self.__range = [int(firstFrame), int(lastFrame)]
            self.__range.sort()
        else:
            self.__range = None

    def getAsString(self):  
        return str(self.__seqstr)
    
    def getDirectory(self): 
        return os.path.dirname(self.__prefix)
        
    def getPrefix(self):    
        return self.__prefix
    
    def getSuffix(self):    
        return self.__suffix
    
    def getPadding(self):   
        return self.__padding

    def getResolvedPath(self, frame):
        if self.__range and (frame < self.__range[0] or frame > self.__range[1]):
            raise IndexError, 'Frame out of range'
        return (_seqfmt % (self.__prefix, self.__padding, self.__suffix)) % frame

    def hasFrameSet(self):     
        return bool(self.__range)

    def hasFrame(self, frame):
        if self.__range:
            return bool(frame>=self.__range[0] and frame<=self.__range[1])
        else:
            return True

    def getFirstFrame(self):
        if self.__range:
            return self.__range[0]
        else:
            return 0

    def getLastFrame(self):
        if self.__range:
            return self.__range[1]
        else:
            return 0

    def getNearestFrames(self, frame):
        left = None
        right = None
        if self.__range:
            if frame > self.__range[0]:
                left = min(frame-1, self.__range[1])
            if frame < self.__range[1]:
                right = max(frame+1, self.__range[0])
        return (left, right)

    def getFrameList(self):
        if self.__range:
            return range(self.__range[0], self.__range[1]+1)
        else:
            return []

# Register the plugin
AssetAPI.RegisterFileSequencePlugin("PyMockFileSeq", MockFileSequencePlugin())
