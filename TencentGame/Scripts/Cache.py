import pdg
import hou
import numpy as np
from PIL import Image
import os
from os import path
from falcon_utils import checkfile, makeoutputpathsafe
import hashlib
import codecs

from pdg import (envVar, strData, intData, floatData, resultData, hasStrData, hasIntData, hasFloatData, hasResultData, resultDataIndex, findResultData, findDirectResultData, floatDataArray, intDataArray, strDataArray, findData, findDirectData, input, workItem, kwargs)

def onGenerate(self, item_holder, upstream_items, generation_type):
    # static parms
    MULTILAYER = ("RGB", "RGBA", "P", "CMYK", "YCbCr", "LAB", "HSV")
    SINGLELAYER = ("1", "L", "I", "F")
    BIT32 = ("I", "F")
    BIT8 = ("L" ,"P", "RGB", "RGBA", "P", "CMYK", "YCbCr", "LAB", "HSV")
    BIT1 = ("1")
    CONVERT32 = 65535.0
    CONVERT8 = 255.0
    node = hou.nodeBySessionId(self.customId)
    isprintlog = node.parm("isprintlog").eval()
    issetattribute = node.parm("setattribute").eval()
    attributeName = node.parm("nofileattribute").evalAsString()
    hasher = hashlib.md5()
    def createHeightFieldGeometry(name, size, center, rotate, heightscale=1, moveheight=0, 
                                  isFileExists=0, filepath="", initval=0):
        '''
        create HeightField Geometry from file
        '''
        # init geometry
        fileerror = False

        geo = hou.Geometry().freeze()
        geo.addAttrib(hou.attribType.Prim, "name", "")

        halfsize = size * 0.5
        bounds = hou.Geometry().boundingBox()
        bounds.setTo((-halfsize, -halfsize, -0.5, halfsize, halfsize, 0.5))
        
        # init volume
        vol = geo.createVolume(size, size, 1, bounds)
        vol.setAttribValue("name", name)
        im_hash = "-1"
        if isFileExists:
        # read landscape/masks
            try:
                im = Image.open(filepath)
                im = im.transpose(Image.TRANSPOSE)
                # check image size
                if im.size[0] != size or im.size[1] != size:
                    im = im.resize((size, size))
                # save as image numpy array pix
                im_hash = hashlib.md5(im.tobytes()).hexdigest()
                im_hash = str(im_hash)
                immode = im.mode
                if immode in SINGLELAYER:
                    if immode in BIT32:
                        pix = np.asarray(im) / CONVERT32
                    elif immode in BIT8:
                        pix = np.asarray(im) / CONVERT8
                    else: 
                        pix = np.asarray(im)
              #  im_hash = str(im_hash)

            except:
                fileerror = True
                pix = np.full((size, size), initval)
        else:
            pix = np.full((size, size), initval)
        # set pix to volume
        vol.setAllVoxels((pix*heightscale+moveheight).flatten())            
        # transform geometry
        rotationMtx = hou.hmath.buildRotate(-90,-90,0)
        transformMtx = hou.hmath.buildTranslate(center[0],0,center[1])
        geo.transform(rotationMtx)
        geo.transform(transformMtx)

        return geo, fileerror, im_hash

    E = lambda p : node.parm(p).eval()
    # gather parameters
    size = node.parm("size").evalAsInt()
    filepath = node.parm("filepath").evalAsString()
    center = node.parmTuple('t').eval()
    rotate = E('rotate')
    heightscale = E('heightscale')
    moveheight = E('moveheight')

    iffilenotexist = E('iffilenotexist') # 0 -> report error
                                                        # 1 -> set black
                                                        # 2 -> set white
                                                        # 3 -> set value
    setvalue = E('setvalue')             # only if iffilenotexist == 3                                           
    loadtype = E("loadtype")   # 0 -> create heightfield from scrach 
                                                # 1 -> load to upstream heightfield
    createtype = E("type")     # 0 -> heightfield, 
                                                # 1 -> mask
    layername = E("layer")     # if load to upstream heightfield, set custom mask name
    layermode = E("layermode") # 0 -> replace
                                                # 1 -> add
                                                # 2 -> subtract
                                                # 3 -> difference
                                                # 4 -> multiply
                                                # 5 -> maximum
                                                # 6 -> minimum

    bordertype = E("layerborder") # 0 -> constant
                                                    # 1 -> repeat
                                                    # 2 -> streak

    borderval = E("layerborderval") # only if bordertype == 0

    is_set_hash = E("issethash")
    hash_attrib = E("hashattributename")
    is_add_hash = E("isaddhash")
    add_hash_attrib = E("addhashattrib")
    for upstream_item in upstream_items:     
        fileerror = False

        # check if file exist
        isfileexists = checkfile(filepath)
        initval = 0

        if not isfileexists:
            if iffilenotexist == 0:
                raise IOError("File not found! Please check your file {0}".format(filepath))
            elif iffilenotexist == 2:
                initval = 1
            elif iffilenotexist == 3:
                initval = setvalue

        # do works
        ## node verb for border type (primitive node)
        maskprimitiveVerb = createVolumePrimitiveVerb("mask")
        im_hash = -1
        if loadtype == 0:
            heightfieldpeimitiveVerb = createVolumePrimitiveVerb("height")
            if createtype == 0:
                name = "height"
                heightfield, fileerror, im_hash = createHeightFieldGeometry(name, size, center, rotate, heightscale, moveheight, isfileexists, filepath, initval)
                masklayer, maskerror, _ = createHeightFieldGeometry("mask", size, center, rotate)
            else:
                name = "mask"
                masklayer, fileerror, im_hash = createHeightFieldGeometry(name, size, center, rotate, heightscale, moveheight, isfileexists, filepath, initval)
                heightfield, maskerror, _ = createHeightFieldGeometry("height", size, center, rotate)
            
            heightfieldpeimitiveVerb.execute(heightfield, [heightfield])
            maskprimitiveVerb.execute(masklayer, [masklayer])

            heightfield.merge(masklayer)
            
        else:
            heightfield = hou.Geometry().freeze()
            heightfield.loadFromFile(upstream_item.resultData[0][0])
            name = layername
            additionallayer, fileerror, im_hash = createHeightFieldGeometry(name, size, center, rotate, heightscale, moveheight, isfileexists, filepath, initval)
            additionalPrimitiveVerb = createVolumePrimitiveVerb(name, bordertype, borderval)
            additionalPrimitiveVerb.execute(additionallayer, [additionallayer])

            ismergelayer = 0
            for layer in heightfield.prims():
                if layer.stringAttribValue("name") == name:
                    ismergelayer = 1
                    voxels = additionallayer.prims()[0]
                    if layermode == 0:                # 0 -> replace
                        layer.setAllVoxels(voxels.allVoxels())
                    else:
                        in_voxelArray = np.asanyarray(layer.allVoxels)
                        blend_voxelArray = np.asanyarray(voxels.allVoxels)
                        if layermode == 1:                # 1 -> add
                            layer.setAllVoxels((in_voxelArray+blend_voxelArray).flatten())
                        elif layermode == 2:              # 2 -> subtract
                            layer.setAllVoxels((in_voxelArray-blend_voxelArray).flatten())
                        elif layermode == 3:              # 3 -> difference
                            layer.setAllVoxels(np.absolute(in_voxelArray-blend_voxelArray).flatten())
                        elif layermode == 4:              # 4 -> multiply
                            layer.setAllVoxels((in_voxelArray*blend_voxelArray).flatten())
                        elif layermode == 5:              # 5 -> maximum
                            layer.setAllVoxels(np.maximum(in_voxelArray+blend_voxelArray).flatten())
                        elif layermode == 6:              # 6 -> minimum
                            layer.setAllVoxels(np.minimum(in_voxelArray+blend_voxelArray).flatten())
            if ismergelayer == 0:
                heightfield.merge(additionallayer)
        ## node verb for visulization (volumevisualization node)
        heightfieldVisVerb = createHeightFieldVisVerb()
        heightfieldVisVerb.execute(heightfield, [heightfield])
        
        work_item = item_holder.addWorkItem(cloneResultData=False, preserveType=True,
            parent=upstream_item)

        if is_set_hash:
            if is_add_hash:
                im_hash = "%x" % (int(im_hash, 16) + int(add_hash_attrib, 16))
            work_item.data.setString(hash_attrib, im_hash, 0)
            outputfile = node.parm('outputfile').unexpandedString()
            outputfile = outputfile.replace("`@{0}`".format(hash_attrib), im_hash)
            outputfile = hou.expandString(outputfile)
        else:
            outputfile = E('outputfile') 
        # self.scheduler.localizePath(work_item.resultData[0][0])
        makeoutputpathsafe(outputfile)
        heightfield.saveToFile(outputfile)
        work_item.addResultData(outputfile, "file/geo", 0)

        if not isfileexists:
            if isprintlog:
                print "File not found: {0}".format(filepath)
            if issetattribute:
                work_item.data.setInt(attributeName, 1, 0)
        else:
            if issetattribute:
                if fileerror:
                    work_item.data.setInt(attributeName, 1, 0)
                else:
                    work_item.data.setInt(attributeName, 0, 0)
        return pdg.result.Success

def onRegenerateStatic(self, item_holder, existing_items, upstream_items):
    return pdg.result.Success

def remap(pixelArray, out_range, from_range=(0,255), to_range=(0,255)):
    if out_range == 0:
        return pixelArray * 65535
    elif out_range == 1:
        return np.interp(pixelArray, (pixelArray.min(), pixelArray.max()), to_range)
    else:
        return np.interp(pixelArray, from_range, to_range)

def createVolumePrimitiveVerb(volumeType, bordertype=0, borderval=0):

    primitiveVerb = hou.sopNodeTypeCategory().nodeVerb("primitive")
    if volumeType == "height":
        primitiveVerb.setParms({
                                "dovolvis":1,
                                "volvis":4,
                                "dovolume":1,
                                "volborder":2
        })
    elif volumeType == "mask":
        primitiveVerb.setParms({
                                "dovolvis":1,
                                "volvis":0,
                                "dovolume":1,
                                "volborder":0
        })
    else:
        primitiveVerb.setParms({
                                "dovolvis":1,
                                "volvis":0,
                                "dovolume":1,
                                "volborder":bordertype,
                                "volborderval":borderval
        })

    return primitiveVerb

def createHeightFieldVisVerb():
    volvizVerb = hou.sopNodeTypeCategory().nodeVerb("volumevisualization")
    lin = hou.rampBasis.Linear
    rampmaskcolor = hou.Ramp((lin,lin), (0,1),((1.0,1.0,1.0), (1.0,0.0,0.0)))
    rampheightfield = hou.Ramp((lin,lin), (0,1),((1.0,1.0,1.0), (1.0,1.0,1.0)))
    volvizVerb.setParms({
                            "vismode":0,
                            "densityfield":"height",
                            "densityramp":rampheightfield,
                            "cdfield":"mask",
                            "cdramp":rampmaskcolor
    })

    return volvizVerb
    
def updateparms(kwargs):
    node = kwargs["node"]
    value = kwargs["parm"].eval()
    if value == 0:
        node.parm("heightscale").set(512)
        node.parm("moveheight").set(-256)
    else:
        node.parm("heightscale").set(1)
        node.parm("moveheight").set(0)



