#!/bin/sh
# .bashrc

if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

PS1='\[\e[1;32m\][\t]\[\e[m\]\[\e[1;31m\]\u\[\e[m\]\[\e[1;33m\]@\[\e[m\]\[\e[1;35m\]\H\[\e[m\]:\[\e[1;34m\]\w\[\e[m\]\[\e[1;32m\]\$\[\e[m\] '
HISTTIMEFORMAT="[`whoami`]%F %T: "
source /mnt/public/Share/john/share/lca_path

# "------rv and shotgun"

export RV_PATHSWAP_ROOT=/mnt/proj
export SHOTGUN_PLUGIN_PROTOCOL_RESTRICTION="http"
export SHOTGUN_PLUGIN_DOMAIN_RESTRICTION="shotgun.zhuiguang.com"
export PATH=/usr/local/rv/rv-Linux-x86-64-4.0.10/bin:$PATH

# "------mtoa"
export MTOA_HOME=/mnt/proj/software/develop/maya/arnold/MtoA_1.2.4.2_linux_2015-lca
export MAYA_MODULE_PATH=$MTOA_HOME
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export PYTHONPATH=$MTOA_HOME/scripts:$PYTHONPATH
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export solidangle_LICENSE=6053@10.0.0.5
export ALSHADERS_HOME=/mnt/proj/software/develop/maya/utility/devShaders
export MTOA_TEMPLATES_PATH=$ALSHADERS_HOME/ae
export MAYA_CUSTOM_TEMPLATE_PATH=$ALSHADERS_HOME/aexml
export ARNOLD_PLUGIN_PATH=$ALSHADERS_HOME/bin

export DEVSHADERS_HOME=/home/xukai/Documents/MAYA/devShaders
export MTOA_TEMPLATES_PATH=$DEVSHADERS_HOME/ae:$MTOA_TEMPLATES_PATH
export MAYA_CUSTOM_TEMPLATE_PATH=$DEVSHADERS_HOME/aexml:$MAYA_CUSTOM_TEMPLATE_PATH
export ARNOLD_PLUGIN_PATH=$DEVSHADERS_HOME/bin:$ARNOLD_PLUGIN_PATH

# "------common env"
export LCTOOLSET=/mnt/utility/toolset
export LCADEVLIB=/mnt/proj/software/lib
export PYTHONPATH=$LCTOOLSET/lib:$LCTOOLSET/lib/3rd_party:$LCADEVLIB/lib64/python2.6/site-packages:$LCTOOLSET/tools:$PYTHONPATH
export LD_LIBRARY_PATH=$LCADEVLIB/lib:$LCADEVLIB/ilmbase1.6/lib:$LD_LIBRARY_PATH

# "-----maya"
export PYTHONPATH=$LCTOOLSET/applications/maya/scripts:$LCTOOLSET/applications/maya/2015-x64/scripts:/mnt/utility/lca_rig:$PYTHONPATH
export MAYA_SCRIPT_PATH=$LCTOOLSET/applications/maya/scripts:$MAYA_SCRIPT_PATH
export MAYA_PLUG_IN_PATH=$LCTOOLSET/applications/maya/2015-x64/plugins:$MAYA_PLUG_IN_PATH
export MAYA_SHELF_PATH=$LCTOOLSET/applications/maya/shelves:$MAYA_SHELF_PATH


# qualoth
export MAYA_PLUG_IN_PATH=$MAYA_PLUG_IN_PATH:/mnt/usr/fxgear/qualoth/bin/maya2015
export MAYA_SCRIPT_PATH=$MAYA_SCRIPT_PATH:/mnt/usr/fxgear/qualoth/script
export QUALOTH_SERVER=10.0.0.5
export FXGEAR_SERVER=10.0.0.5

# "-----xgen"
export LD_LIBRARY_PATH=/mnt/usr/autodesk/maya2015/plug-ins/xgen/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/mnt/usr/autodesk/maya2015/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=$MTOAHOME/procedurals:${LD_LIBRARY_PATH}
export XGEN_LOCATION=/mnt/usr/autodesk/maya2015/plug-ins/xgen/
export XGEN_CONFIG_PATH=/mnt/usr/autodesk/maya2015/plug-ins/xgen

# "-----Yeti"
YETI_HOME=/mnt/usr/yeti/Yeti-v2.0.19_Maya2015-linux64
export YETI_DOCS=$YETI_HOME/docs
export ARNOLD_PLUGIN_PATH=$YETI_HOME/bin:$YETI_HOME/plug-ins:$ARNOLD_PLUGIN_PATH
export LD_LIBRARY_PATH=$YETI_HOME/bin:$YETI_HOME/plug-ins:$LD_LIBRARY_PATH
export MTOA_EXTENSIONS_PATH=$YETI_HOME/plug-ins:$MTOA_EXTENSIONS_PATH
export MTOA_PROCEDURAL=$YETI_HOME/bin:$MTOA_PROCEDURAL
export PYTHONPATH=$YETI_HOME/scripts:$PYTHONPATH
export PATH=$YETI_HOME/bin:$PATH
export MAYA_MODULE_PATH=/mnt/usr/yeti/Yeti-v2.0.19_Maya2015-linux64:$MAYA_MODULE_PATH
#export MAYA_SCRIPT_PATH=$YETI_HOME/scripts:$MAYA_SCRIPT_PATH
export peregrinel_LICENSE=5059@10.0.0.5
export YETI_TMP=/home/yeti_cache
#abcResample 
export MAYA_PLUG_IN_PATH=/mnt/public/Share/zhixiang/abcResample:$MAYA_PLUG_IN_PATH

# Miarmy in  mayapy
export PYTHONPATH=$PYTHONPATH:/mnt/usr/Basefount/Miarmy/maya/scripts

# scenegraphXML
export PYTHONPATH=/home/xukai/Documents/KT_Libs/python:$PYTHONPATH

