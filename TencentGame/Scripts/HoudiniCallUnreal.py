import hou
import pdg
import httplib
import time
from pdg import (envVar, strData, intData, floatData, resultData, hasStrData, hasIntData, hasFloatData, hasResultData, resultDataIndex, findResultData, findDirectResultData, floatDataArray, intDataArray, strDataArray, findData, findDirectData, input, workItem, kwargs)



def onGenerate(self, item_holder, upstream_items, generation_type):
    node = hou.nodeBySessionId(self.customId)
    DEFAULT_TIMEOUT = 10

    portnum = node.parm("portnum").evalAsInt()
    DEFAULT_SLEEP = node.parm("sleep").eval()
    E = lambda pn: node.parm(pn).eval()
    S = lambda pn: node.parm(pn).evalAsString()
    kill = node.parm("kill")
    isExtraCommand = E("isextra")
    command1 = S("integrationway")
    command2 = S("integrationtype")
    command3 = S("integrationtile")
    extracommand = S("extracommand")
    CONNECTTIMES = E("connecttimes")
    isKeepResult = E("resultdata")
    if command3 == "none":
        _command3 = ""
    else:
        _command3 = command3
        
    if _command3:
        for item in upstream_items:
            x_coord = intData(item, "x_coord", 0)
            y_coord = intData(item, "y_coord", 0)
            coord = '_'.join(["x"+str(x_coord), "y"+str(y_coord)])
            _command3 = command3.replace("xx_yy", coord)
            if isKeepResult:
                item_holder.addWorkItem(cloneResultData=True, preserveType=True, parent=item)
            else:
                work_item = item_holder.addWorkItem(index=item.index, parent=item)
            
            # send common
            if not _command3.startswith("&fbxpath"):
                if not isExtraCommand or extracommand == "none":
                    connectToUE4(portnum, kill, DEFAULT_TIMEOUT, DEFAULT_SLEEP, CONNECTTIMES, command1, command2, _command3)
                else:
                    if extracommand.endswith(".geo"):
                        _extracommand = extracommand
                        connectToUE4(portnum, kill, DEFAULT_TIMEOUT, DEFAULT_SLEEP, CONNECTTIMES, command1, command2, _command3, _extracommand)
                    # spawn points
                    else:
                        folder = S("actorfolder")
                        if extracommand.startswith("&table"):
                            pointCloudGeo = S("pointcloudgeo")
                            pointCloudLabel = S("pointcloudlabel")
                            _extracommand = "&table=" + pointCloudGeo + "&actorlabel=" + pointCloudLabel + "&actorfolder=" + folder
    
                            connectToUE4(portnum, kill, DEFAULT_TIMEOUT, DEFAULT_SLEEP, CONNECTTIMES, command1, command2, _command3, _extracommand)

            # send fbx
            else:
                if extracommand == "none":
                    raise IOError("wrong parameters for fbx exporting to UE4")

                else:
                    fbx_name = S("fbxfilename")
                    fbx_name_ext = S("fbxfilenameext")
                    material_name = S("materialname")
                    uasset_path = S("uasset")
                    _command3 = _command3.replace("XXX.fbx", fbx_name_ext)
                    _extracommand = extracommand
                    # is fbx output
                    if _extracommand.startswith("&uassetpath"):
                        _extracommand = "&uassetpath=" + uasset_path

                        connectToUE4(portnum, kill, DEFAULT_TIMEOUT, DEFAULT_SLEEP, CONNECTTIMES, command1, command2, _command3, _extracommand)
                
    else:
        connectToUE4(portnum, kill, DEFAULT_TIMEOUT, DEFAULT_SLEEP, CONNECTTIMES, command1, command2, _command3)
        item_holder.addWorkItem(index=0)
            
    return pdg.result.Success


def connectToUE4(portnum, kill, DEFAULT_TIMEOUT, DEFAULT_SLEEP, CONNECTTIMES, *args):
    _waittime = 0
    command1 = args[0]
    command2 = args[1]
    command3 = args[2]

    if len(args)>3:
        extracommand = args[3]
    else:
        extracommand = ""    
    httpConn = httplib.HTTPConnection("localhost", portnum, timeout=DEFAULT_TIMEOUT)
    status = "wip"
    if extracommand:
        commands = '/'+''.join([command1, command2, command3, extracommand])
    else:
        commands = '/'+''.join([command1, command2, command3])
    httpConn.request("GET", commands)
    httpRes = httpConn.getresponse()
    workId = httpRes.read()
    
    while status == "wip":
        if kill.eval():
            raise IOError("User Killed")
            break
        httpConn2 = httplib.HTTPConnection("localhost", 52066, timeout=DEFAULT_TIMEOUT)
        httpConn2.request("GET", "/status?id=" + workId)
        httpRes2 = httpConn2.getresponse()
        status = httpRes2.read()
        _waittime += 1
        print _waittime
        print "Get " + commands + " " + status + " WorkId: " + workId
        if _waittime > CONNECTTIMES:
            raise IOError("connect to UE4 failed")
            break
        time.sleep(DEFAULT_SLEEP)


def onRegenerateStatic(self, item_holder, existing_items, upstream_items):
    return pdg.result.Success
