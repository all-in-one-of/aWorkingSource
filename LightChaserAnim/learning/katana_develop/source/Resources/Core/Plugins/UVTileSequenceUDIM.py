# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

import UVTileSequenceBase
import AssetAPI,os
import re

class UDIMTileSequence(UVTileSequenceBase.UVTileSequenceBase):
    def __init__(self, fileNames=[]):
        self.__fileNames = []
        self.setTextures(fileNames)
            
    def setTextures(self, fileNames):
        """ 
        Specifies the textures to be used in the UV viewer.
        File ranges (eg. diffuse.(1001-1020)#.tx) will be expanded
        As will non-limited file ranges (eg. diffuse.#.tx) 
        """
        del self.__fileNames[:]
        
        # insert individual files into self.__fileNames
        for file in fileNames:
            if file == None:
                continue
            
            fsPlugin = AssetAPI.GetDefaultFileSequencePlugin()
            if fsPlugin.isFileSequence(file):
                fs = fsPlugin.getFileSequence(file)
                frames = fs.getFrameList()
                if len(frames) > 0:
                    # Limited frame range
                    for frame in frames:
                        path = fs.getResolvedPath(frame)
                        if os.path.exists(path):
                            self.__fileNames.append(path)
                else:
                    # Unlimited frame range 
                    paths = self.__getPathsFromUnlimitedSequence(file)
                    self.__fileNames.extend(paths)
                            
            else:
                self.__fileNames.append(file)
    
    def getTextures(self):
        return self.__fileNames
    
    def getTilePositions(self):
        """ Returns file paths and offsets for each texture based on UDIM format
            Returns { filename : (x_position, y_position) }
        """
        fileOffsets = {}
        
        for file in self.__fileNames:
            udim = self.__getUDIM(file)
            x = (udim -1) % 10
            y = int((udim - 1000 - 1) / 10)
            fileOffsets[file] = (x,y)
        
        return fileOffsets
    
    def getTileId(self, filename):
        """ Returns the unique ID of the passed filename.
            "defuseimage.1004.tif" would return "1004"
        """
        _filere = re.compile(r'.*\.(\d+)\..*')
        match = _filere.match(filename)
        if match:
            return match.groups()[0]
       
        return None
    
    def __getPathsFromUnlimitedSequence(self, path):
        fileNames = []
        
        _filere = re.compile(r'(.*\.)\#(\..*)')
        match = _filere.match(path)
        
        if match:
            dir = os.path.dirname(path)
            filename = match.groups()[0]
            fileext = match.groups()[1]
            
            pattern = re.compile(r'(.*' +filename +')(\d+)('+fileext+')')
                                
            for f in os.listdir(dir):
                filepath = os.path.join(dir, f)
                if re.search(pattern, filepath):
                    fileNames.append(filepath)
                    
        return fileNames
        
    def __getUDIM(self, path):
        _filere = re.compile(r'(.*\.)(\d+)(\..*)')
        match = _filere.match(path)
        if match:
            return int(match.groups()[1])
        else:
            # Not a UDIM string - default to 0,0
            return 1001 

  
PluginRegistry = [
    ("UVTileFormats", 1, "UDIM", UDIMTileSequence),
]
