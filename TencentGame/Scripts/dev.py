# Custom onGenerate callback logic. Called when the node should generate new
# work items from upstream static or dynamic work items.

# The following global variables are available:
# 
# self             -   A reference to the current pdg.Node instance
# item_holder      -   A pdg.WorkItemHolder for constructing and adding work items
# upstream_items   -   The list of work items in the node above, or empty list if there are no inputs
# generation_type  -   The type of generation, e.g. pdg.GenerateType.Static, Dynamic, or Regenerate
import hou
from PIL import Image
import os,sys
import numpy as np
def createGeometry(name, size, voxel_size, center, rotate, filepath="", heightscale=512, moveheight=-256, initval=0):
    '''
    create HeightField Geometry from file
    '''
    # init geometry
    import hou
    fileerror = False

    geo = hou.Geometry().freeze()
    geo.addAttrib(hou.attribType.Prim, "name", "")

    halfsize = size * 0.5 * voxel_size
    bounds = hou.Geometry().boundingBox()
    bounds.setTo((-halfsize, -halfsize, -voxel_size/2.0, halfsize, halfsize, voxel_size/2.0))
    
    # init volume
    vol = geo.createVolume(size, size, 1, bounds)
    vol.setAttribValue("name", name)
    # read landscape/masks
    im = Image.open(filepath)
    im = im.transpose(Image.TRANSPOSE)
    # check image size
    if im.size[0] != size or im.size[1] != size:
        im = im.resize((size, size))
    # save as image numpy array pix
    immode = im.mode
    if immode in ("I", "F"):
        pix = np.asarray(im) / 65535.0
    elif immode in ("L" ,"P", "RGB", "RGBA", "P", "CMYK", "YCbCr", "LAB", "HSV"):
        pix = np.asarray(im) / 255.0
    else: 
        pix = np.asarray(im)

    # set pix to volume
    vol.setAllVoxels((pix*heightscale+moveheight).flatten())            
    # transform geometry
    rotationMtx = hou.hmath.buildRotate(-90,-90,0)
    transformMtx = hou.hmath.buildTranslate(center[0],0,center[1])
    geo.transform(rotationMtx)
    geo.transform(transformMtx)

    return geo
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
for i in range(0, 16):
    x = int(i/4);y = int(i%4);coord = "x%i_y%i"%(x, y)
    filepath = "$HIP/wd/$LEVELNAME/H2U/%s/Cache_Terrain/Landscape_gt.png"%coord
    name = "height"
    tilesize = 1009.0;voxelsize = 0.5 
    pos_x = (-3+2*int(i/4))*((tilesize-1.0)/(2/voxelsize))
    pos_y = (-3+2*int(i%4))*((tilesize-1.0)/(2/voxelsize))
    center = (pos_x, pos_y)
    filepath = hou.expandString(filepath)
    heightfield = createGeometry(name, 1009, 0.5,center, 90, filepath)
    geo_path = "$HIP/geo/Landscape_GT_%s.bgeo.sc"%coord
    heightfieldpeimitiveVerb = createVolumePrimitiveVerb("height")
    heightfieldpeimitiveVerb.execute(heightfield, [heightfield])
    heightfieldVisVerb = createHeightFieldVisVerb()
    heightfieldVisVerb.execute(heightfield, [heightfield])
    heightfield.saveToFile(hou.expandString(geo_path))
    workitem = item_holder.addWorkItem(index=i, parent=None)
    print geo_path
    workitem.addResultData(hou.expandString(geo_path), "file/geo", 0)
