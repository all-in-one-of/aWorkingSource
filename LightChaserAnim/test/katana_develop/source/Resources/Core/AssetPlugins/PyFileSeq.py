# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

import os, os.path, sys, re
import AssetAPI

_seqre = re.compile(r'(?P<prefix>.*[\.\-\_])'
                    r'(?P<range>'
                    r'\(\s*(?P<firstFrame>\d+)\s*-\s*(?P<lastFrame>\d+)\s*\))?'
                    r'(?P<format>(%(?P<printfSyntaxPadding>\d+)d)|#{1,})'
                    r'(?P<suffix>\..*)')
_fileExpression = re.compile(r'(?P<prefix>.*[\.\-\_])'
                             r'(?P<number>\d{1,10})'
                             r'(?P<suffix>\..*)')

_seqfmt = '%s%%0%dd%s'                  # image.%04d.exr
_seqfmtFirstLast = '%s(%d-%d)%%0%dd%s'  # image.(1-10)%04d.exr


'''
This File Sequence Plugin uses a Nuke-like file sequence format. For a
4 zero-padded file sequence the format would be, for example:
  image.%04d.exr

Additionally, a number of n hashes is supported for padding, e.g.: image.#.exr
for a single digit (image.1.exr), image.##.exr for two digits (image.01.exr),
etc.
'''


class FileSequencePlugin(AssetAPI.BaseFileSequencePlugin):
    '''
    The file sequence plugin class.
    '''
    def getFileSequence(self, seqstr):
        return FileSequence(seqstr)

    def isFileSequence(self, seqstr):
        return bool(_seqre.match(seqstr))

    def buildFileSequenceString(self, prefix, suffix, padding,
                                first = None, last = None):
        if first is not None and last is not None:
          ret = _seqfmtFirstLast % (prefix, first, last, padding, suffix)
        else:
          ret = _seqfmt % (prefix, padding, suffix)

        return ret

    def findSequence(self, fileList):
        fileList.sort() # Ensure list is sorted
        seq = set()
        nonseq = []
        mapFirstFrame = {}
        mapLastFrame = {}
        internalSeqCount = 0
        for filename in fileList:
            match = _fileExpression.match(filename)
            if match:
                g = match.groupdict()

                seqId = g['prefix'] + g['suffix'] + str(internalSeqCount)
                nb = int(g['number'])

                # Split sequences if sequence is interrupted,
                # e.g. i.05.jpg -> i.08.jpg
                if mapLastFrame.has_key(seqId) and mapLastFrame[seqId]+1 != nb:
                    internalSeqCount += 1
                    seqId = g['prefix'] + g['suffix'] + str(internalSeqCount)

                if seqId in mapFirstFrame:
                    if mapFirstFrame[seqId] == mapLastFrame[seqId]:
                        # As soon as there is another file, we consider it a
                        # sequence. Add to seq and remove the item from nonseq.
                        seq.add( (seqId, g['prefix'], g['suffix'],
                                  len(g['number'])) )
                        nonseq.pop()

                    # In map already, increment last frame
                    mapLastFrame[seqId] = nb

                else:
                    # New sequence
                    mapFirstFrame[seqId] = nb
                    mapLastFrame[seqId] = nb

                    # Add it to nonseq because we don't know if the it will
                    # be a sequence or just one file
                    nonseq.append(filename)

            else:
                nonseq.append(filename)

        seq = tuple([FileSequence(self.buildFileSequenceString(x[1], x[2], x[3],
                        mapFirstFrame[x[0]], mapLastFrame[x[0]])) for x in seq])

        return seq, nonseq

class FileSequence(object):
    '''
    The class that implements the File Sequence.
    '''
    def __init__(self, seqstr):

        self.__seqstr = seqstr
        seqm = _seqre.match(self.__seqstr)
        if not seqm:
            raise ValueError, 'Not a valid sequence'
        g = seqm.groupdict()

        if g['printfSyntaxPadding']:
            # Printf syntax, e.g. %04d
            self.__padding = int(g['printfSyntaxPadding'])
        else:
            # Hash syntax
            self.__padding = len(g['format'])

        self.__prefix = g['prefix']
        self.__suffix = g['suffix']

        firstFrame = g['firstFrame']
        lastFrame = g['lastFrame']
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
AssetAPI.RegisterFileSequencePlugin("FileSeq", FileSequencePlugin())
