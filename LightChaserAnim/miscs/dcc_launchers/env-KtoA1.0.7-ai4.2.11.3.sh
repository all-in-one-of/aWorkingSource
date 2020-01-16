#!/usr/bin/env bash

##lca katana lanucher env
##define env KATANA_HOME for the specified katna version
##define env LC_TOOLSET for the specified lcatools version


##if no LC_PROJ_PATH,set it to /mnt/proj/projects
if [ -z $LC_PROJ_PATH ];then
    export LC_PROJ_PATH=/mnt/proj/projects
fi

if [ -z $KATANA_HOME ];then
    export KATANA_HOME=/mnt/proj/software/k2/katana2.1v1
fi

##in case too many open files
ulimit -HSn 65536 &>/dev/null

export LCTOOLSET=${LC_TOOLSET}
export LGT_RESOURCE=/mnt/proj/resource/lgt

#solid angle
export solidangle_LICENSE=6053@10.0.0.5
export ARNOLD_SHADERLIB_PATH=/mnt/proj/software/k2/shaderlibs_k222
export KTOA_HOME=/mnt/proj/software/k2/KtoA-1.0.7-linux-lca
#export KTOA_HOME=/home/xukai/Documents/TsuiLibs/KtoALibs/KtoA-1.0.8-linux-lca
export ARNOLD_SHADERLIB_PATH=/home/xukai/Documents/TsuiLibs/ShaderLib:${ARNOLD_SHADERLIB_PATH}

export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin:/usr/local/rv/rv-Linux-x86-64-4.0.10/bin
export LD_LIBRARY_PATH=${KTOA_HOME}/bin:/mnt/proj/software/muster8.5.7
export PYTHONPATH=${LCTOOLSET}/applications/katana/Scripts:${LGT_RESOURCE}/katana/Scripts:${LCTOOLSET}/tools:/mnt/proj/software/lib/lib64/python2.6/site-packages:/mnt/proj/software/muster8.5.7-sdk/libs/linux64/python27:${LCTOOLSET}/lib
export LUA_PATH=/mnt/utility/toolset/applications/katana_v2/Lua_scripts/?.lua

# mtoa
MTOAHOME=/mnt/usr/Mtoa_v1.2.4.2_linux_2015
export PYTHONPATH=$MTOAHOME/scripts:$PYTHONPATH
export MAYA_RENDER_DESC_PATH=$MTOAHOME

# katana env
export DEFAULT_RENDERER=arnold
export KATANA_RESOURCES=${KTOA_HOME}:${KATANA_HOME}/plugins/Resources/Examples:/mnt/proj/software/k2/resources_k22:$LGT_RESOURCE/katana_v2:${LCTOOLSET}/applications/katana_v2

# yeti procedural
YETI_HOME=/mnt/usr/yeti/Yeti-v2.0.19_Maya2015-linux64
export PATH=$YETI_HOME/bin:$PATH
export peregrinel_LICENSE=5059@10.0.0.5

# xgen procedural
MAYA_ROOT_PATH=/mnt/usr/autodesk/maya2016
export LD_LIBRARY_PATH=${MAYA_ROOT_PATH}/plug-ins/xgen/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${MAYA_ROOT_PATH}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/mnt/usr/Mtoa_v1.2.4.2_linux_2015/procedurals:${LD_LIBRARY_PATH}
export XGEN_LOCATION=/mnt/usr/autodesk/maya2015-x64-sp6/plug-ins/xgen/
export XGEN_CONFIG_PATH=/mnt/usr/autodesk/maya2015-x64-sp6/plug-ins/xgen/

# shotgun
export PYTHONPATH=/mnt/work/soft/tank/studio/install/core/python:${PYTHONPATH}
export KATANA_RESOURCES=${TANK_KATANA_RESOURCES}:${KATANA_RESOURCES}

# boost
export LD_LIBRARY_PATH=/mnt/proj/software/lib/shared_libraries/boost_1.56:${LD_LIBRARY_PATH}

# OpenEXR
export LD_LIBRARY_PATH=/mnt/proj/software/lib/shared_libraries/openexrlib-2.2:${LD_LIBRARY_PATH}

# OpenVDB
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/proj/software/lib/OpenVDB/lib:/mnt/usr/hfs15/dsolib