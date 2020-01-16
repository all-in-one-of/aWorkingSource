# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

import UVTileSequenceBase
import AssetAPI,os
import re

class MudboxTileSequence(UVTileSequenceBase.UVTileSequenceBase):
    def __init__(self, fileNames=[]):
        self.__fileNames = []
        self.setTextures(fileNames)
   
            
    def setTextures(self, fileNames):
        """ 
        Specifies the textures to be used in the UV viewer.
        File ranges (eg. diffuse_#.tx) will be expanded to
        diffuse_u1_v1.tx, diffuse_u2_v1.tx, diffuse_u3_v1.tx, diffuse_u1_v2.tx ...
        """
        del self.__fileNames[:]
        
        # insert individual files into self.__fileNames
        for file in fileNames:
            if file == None:
                continue
           
            # Is this a valid mudbox format 
            _filere = re.compile(r'(.*_\#.*)', re.IGNORECASE)
            match = _filere.match(file)
            if match:
                # A mudbox file name range 
                paths = self.__getPathsFromSequence(file)
                self.__fileNames.extend(paths)
            else:
                # A single file name
                self.__fileNames.append(file)
           
    def getTextures(self):
        return self.__fileNames

    def getTilePositions(self):
        """ Returns file paths and offsets for each texture based on UDIM format
            Returns { filename : (x_position, y_position) }
        """
        fileOffsets = {}
        
        for file in self.__fileNames:
            fileOffsets[file] = self.__getOffset(file)
        
        return fileOffsets
    
    def getTileId(self, filename):
        """ Returns the unique ID of the passed filename.
            defuseimage_u2_v1.tif would return "u2_v1"
        """
        _filere = re.compile(r'.*_(u\d+_v\d+).*', re.IGNORECASE)
        match = _filere.match(filename)
        if match:
            return match.groups()[0]
        
        return None
        
    
    def __getPathsFromSequence(self, path):
        fileNames = []
        
        _filere = re.compile(r'(.*_)\#(\..*)')
        match = _filere.match(path)
        
        if match:
            dir = os.path.dirname(path)
            filename = match.groups()[0]
            fileext = match.groups()[1]
            
            pattern = re.compile(r'.*' +filename +'u\d+_v\d+.*'+fileext, re.IGNORECASE)
            for f in os.listdir(dir):
                filepath = os.path.join(dir, f)
                if re.search(pattern, filepath):
                    fileNames.append(filepath)

        return fileNames
        
    def __getOffset(self, path):
        # Check for a file containing a valid mudbox string eg. "texturename_u1_v5.tiff" 
        _filere = re.compile(r'(.*_u\d+_v\d+.*)', re.IGNORECASE)
        match = _filere.match(path)
        if match:
            """ 
            Split the fiel name to retrive the U & V positions so the
            above example will become: ['1', '5',]
            """
            splitre = re.compile(r'.*_u(\d+)_v(\d+).*', re.IGNORECASE)
            split = splitre.match(path)
            if split:
               u = int(split.groups()[0])
               v = int(split.groups()[1])
               
               return (u-1,v-1)
            else:
                return (0,0)
        else:
            # Not a mudbox string - default to 0,0
            return (0,0) 


PluginRegistry = [
    ("UVTileFormats", 1, "Mudbox", MudboxTileSequence),
]
