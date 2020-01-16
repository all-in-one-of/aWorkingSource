# Executes a Python script, either in process or as a job
#
# The following variables and functions are available in both case:
# work_item
# intData, floatData, strData,
# intDataArray, floatDataArray, strDataArray
#
# In addition to the above, in-process scripts can also access:
# self         - the current PDG node
# parent_item  - the parent work item, if it exists
import hou
import os,sys
coord = strData(work_item, "coord")
levelname = strData(work_item, "levelname")
levelpath = strData(work_item, "levelpath")

projectpath = hou.expandString("$HIP/wd/%s/H2U/%s"%(levelname, coord))

standard_folders = [
    "Cache_Terrain",
    "Cache_River",
    "Cache_Swamp",
    "Cache_Lake",
    "Cache_Shoal",
    "Cache_Brook",
]

if os.path.isdir(projectpath):    
    # build folder
    for folder_path in standard_folders:
        if os.path.exist(os.path.join(projectpath, folder_path))
            continue
        os.mkdir(folder_path)
