# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

class UVTileSequenceBase():
    '''
    Primary class to derive from when implementing a UV tile sequence plugin.
    '''
    
    def __init__(self, fileNames=[]):
        '''
        Init optionally takes a list of texture file names to use
        '''
        pass
    
    def setTextures(self, filenames):
        '''
        Processes the texture names to be used in the tile sequence. This is 
        where file name ranges are interpreted into discreet file names  
        '''
        pass
    
    def getTextures(self):
        '''
        Returns the full list of discreet texture paths the were detected
        '''
        pass
    
    def getTilePositions(self):
        '''
        Returns a dictionary with the file name as the key and a tuple with the
        U & V offsets as the value
        '''
        pass
    
    def getTileId(self, filename):
        """ Returns the unique ID of the passed filename.
            "defuseimage_u2_v1.tif" would return "u2_v1"
        """
        return None