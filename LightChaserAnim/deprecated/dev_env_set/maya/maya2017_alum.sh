#!/usr/bin/env bash

# ------systrm setting
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi
PS1='\[\e[1;32m\][\t]\[\e[m\]\[\e[1;31m\]\u\[\e[m\]\[\e[1;33m\]@\[\e[m\]\[\e[1;35m\]\H\[\e[m\]:\[\e[1;34m\]\w\[\e[m\]\[\e[1;32m\]\$\[\e[m\] '
HISTTIMEFORMAT="[`whoami`]%F %T: "

if [ -z $LCTOOLSET ];then
    export LCTOOLSET=/mnt/utility/toolset
fi


if [ -z $LC_PROJ_PATH ];then
    export LC_PROJ_PATH=/mnt/proj/projects
fi

if [ -z $KATANA_HOME ];then
    export KATANA_HOME=/mnt/work/software/katana/katana2.5v5
fi

if [ -z $MAYA_HOME ];then
    export MAYA_HOME=/mnt/usr/autodesk/maya2017
fi


if [ -z $LCA_DEVKTN_PATH ];then
    export LCA_DEVKTN_PATH=/mnt/work/software/develop/katana
fi

if [ -z $LCA_DEVMY_PATH ];then
    export LCA_DEVMY_PATH=/mnt/work/software/develop/maya
fi

if [ -z $LCA_DEVUSD_PATH ];then
    export LCA_DEVUSD_PATH=/mnt/work/software/develop/usd
fi

if [ -z $LCA_SOFTWARE_PATH ];then
    export LCA_SOFTWARE_PATH=/mnt/work/software
fi

# ------common env
export KATANA_HOME=${KATANA_HOME}
export MAYA_HOME=${MAYA_HOME}
export LCTOOLSET=${LC_TOOLSET}
export LC_PROJ_PATH=${LC_PROJ_PATH}
export LCA_SOFTWARE_PATH=${LCA_SOFTWARE_PATH}
export LCA_DEVKTN_PATH=${LCA_DEVKTN_PATH}
export LCA_DEVMY_PATH=${LCA_DEVMY_PATH}
export LCADEVLIB="/mnt/work/software/lib"
export PYTHONPATH="$LCTOOLSET/lib:$LCTOOLSET/lib/3rd_party:$LCADEVLIB/lib64/python2.7/site-packages:$LCADEVLIB/lib64/python2.6/site-packages:$LCTOOLSET/tools:$PYTHONPATH"
export LD_LIBRARY_PATH="$LCADEVLIB/lib:$LCADEVLIB/ilmbase1.6/lib:$LD_LIBRARY_PATH"


# ------rv and shotgun
export RV_PATHSWAP_ROOT=/mnt/proj
export SHOTGUN_PLUGIN_PROTOCOL_RESTRICTION="http"
export SHOTGUN_PLUGIN_DOMAIN_RESTRICTION="shotgun.zhuiguang.com"
export PATH=/usr/local/rv/rv-Linux-x86-64-4.0.10/bin:$PATH


# -----maya
export PYTHONPATH=${LCTOOLSET}/applications/maya/scripts:$LCTOOLSET/applications/maya/2017-x64/scripts:/mnt/utility/lca_rig:$PYTHONPATH
export MAYA_SCRIPT_PATH=${LCTOOLSET}/applications/maya/scripts:$MAYA_SCRIPT_PATH
export MAYA_PLUG_IN_PATH=${LCTOOLSET}/applications/maya/2017-x64/plugins:$MAYA_PLUG_IN_PATH
export MAYA_SHELF_PATH=${LCTOOLSET}/applications/maya/shelves:$MAYA_SHELF_PATH


# fix opencl bug
export MAYA_IGNORE_OPENCL_VERSION=1
export MAYA_OPENCL_IGNORE_DRIVER_VERSION=1



# ------mtoa
export MTOA_HOME=${LCA_DEVMY_PATH}/arnold/mtoa-2.0.1.0-2017
export MAYA_MODULE_PATH=$MTOA_HOME
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export PYTHONPATH=${MTOA_HOME}/scripts:$PYTHONPATH
export solidangle_LICENSE=6053@10.0.0.5
# --alShaders
export MTOA_TEMPLATES_PATH=${LCA_DEVKTN_PATH}/shader/5.x/alShaders/AE:$MTOA_TEMPLATES_PATH
export MAYA_CUSTOM_TEMPLATE_PATH=${LCA_DEVKTN_PATH}/shader/5.x/alShaders/AEXml:$MAYA_CUSTOM_TEMPLATE_PATH
export ARNOLD_PLUGIN_PATH=${LCA_DEVKTN_PATH}/shader/5.x/alShaders:$ARNOLD_PLUGIN_PATH
# --lcaShaders
export MTOA_TEMPLATES_PATH=${LCA_DEVKTN_PATH}/shader/5.x/lcaShaders/AE:$MTOA_TEMPLATES_PATH
export MAYA_CUSTOM_TEMPLATE_PATH=${LCA_DEVKTN_PATH}/shader/5.x/lcaShaders/AEXml:$MAYA_CUSTOM_TEMPLATE_PATH
export ARNOLD_PLUGIN_PATH=${LCA_DEVKTN_PATH}/shader/5.x/lcaShaders:$ARNOLD_PLUGIN_PATH
# --oslShaders
export MTOA_TEMPLATES_PATH=${LCA_DEVKTN_PATH}/shader/5.x/oslShaders/AE:$MTOA_TEMPLATES_PATH
export MAYA_CUSTOM_TEMPLATE_PATH=${LCA_DEVKTN_PATH}/shader/5.x/oslShaders/AEXml:$MAYA_CUSTOM_TEMPLATE_PATH
export ARNOLD_PLUGIN_PATH=${LCA_DEVKTN_PATH}/shader/5.x/oslShaders:$ARNOLD_PLUGIN_PATH


# ------yeti
export YETI_HOME=${LCA_DEVMY_PATH}/yeti/yeti-v2.2.0b02-2017
export MAYA_MODULE_PATH=${YETI_HOME}:$MAYA_MODULE_PATH
export MAYA_SCRIPT_PATH=${YETI_HOME}/scripts:$MAYA_SCRIPT_PATH
export YETI_DOCS=${YETI_HOME}/docs
export ARNOLD_PLUGIN_PATH=${YETI_HOME}/bin:$YETI_HOME/plug-ins:$ARNOLD_PLUGIN_PATH
export LD_LIBRARY_PATH=${YETI_HOME}/bin:$LD_LIBRARY_PATH
export MTOA_EXTENSIONS_PATH=${YETI_HOME}/plug-ins:$MTOA_EXTENSIONS_PATH
export MTOA_PROCEDURAL=${YETI_HOME}/bin:$MTOA_PROCEDURAL
export PYTHONPATH=${YETI_HOME}/scripts:$PYTHONPATH
export PATH=${YETI_HOME}/bin:$PATH
export peregrinel_LICENSE=5059@10.0.0.5
export YETI_TMP=/home/yeti_cache


# ------xgen
export LD_LIBRARY_PATH=${MAYA_HOME}/plug-ins/xgen/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${MAYA_HOME}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${MTOA_HOME}/procedurals:${LD_LIBRARY_PATH}
export XGEN_LOCATION=${MAYA_HOME}/plug-ins/xgen
export XGEN_CONFIG_PATH=${MAYA_HOME}/plug-ins/xgen

# ------qualoth
export MAYA_PLUG_IN_PATH=${LCA_DEVMY_PATH}/qualoth/qualoth-4.2-8/bin:$MAYA_PLUG_IN_PATH
export MAYA_SCRIPT_PATH=${LCA_DEVMY_PATH}/qualoth/qualoth-4.2-8/script:$MAYA_SCRIPT_PATH
export QUALOTH_SERVER=10.0.0.5
export FXGEAR_SERVER=10.0.0.5
export FXLM_IPADDR=10.0.0.5

# ------USD
export USD_INSTALL_ROOT=${LCA_DEVUSD_PATH}/lca_usd_house/usd_centos_v075
export MAYA_PLUG_IN_PATH=${USD_INSTALL_ROOT}/third_party/maya/plugin/:$MAYA_PLUG_IN_PATH
export MAYA_SCRIPT_PATH=${USD_INSTALL_ROOT}/third_party/maya/share/usd/plugins/usdMaya/resources/:$MAYA_SCRIPT_PATH
export PYTHONPATH=${USD_INSTALL_ROOT}/lib/python/:$PYTHONPATH
export XBMLANGPATH=${USD_INSTALL_ROOT}/third_party/maya/share/usd/plugins/usdMaya/resources/:XBMLANGPATH

# ------alum
export ALUM_ROOT=${LCA_DEVUSD_PATH}/lca_usd_house/alum_centos_v0202
export MAYA_PLUG_IN_PATH=${ALUM_ROOT}/plugin:$MAYA_PLUG_IN_PATH
export PXR_PLUGINPATH=${ALUM_ROOT}/lib:$PXR_PLUGINPATH
export PYTHONPATH=${ALUM_ROOT}/lib/python:$PYTHONPATH