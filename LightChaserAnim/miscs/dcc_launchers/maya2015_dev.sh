#!/bin/sh
# .bashrc

if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# "------mtoa"
MTOA_HOME=/home/xukai/Documents/MAYA/MtoA/MtoA-1.4.1.2-linux-2015
export MAYA_MODULE_PATH=$MTOA_HOME
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export PYTHONPATH=$MTOA_HOME/scripts:$PYTHONPATH
export MAYA_RENDER_DESC_PATH=$MTOA_HOME
export solidangle_LICENSE=6053@10.0.0.5
export MTOA_TEMPLATES_PATH=/mnt/proj/software/MAYA/Utility/alShaders-linux-1.0.0rc19-ai4.2.12.2/ae
export MAYA_CUSTOM_TEMPLATE_PATH=/mnt/proj/software/MAYA/Utility/alShaders-linux-1.0.0rc19-ai4.2.12.2/aexml
export ARNOLD_PLUGIN_PATH=/mnt/proj/software/MAYA/Utility/alShaders-linux-1.0.0rc19-ai4.2.12.2/bin

# "------redshift"
REDSHIFT_COREDATAPATH=/home/xukai/Documents/MAYA/MtoRS/RedShift-2.0.74
export REDSHIFT_PLUG_IN_PATH=$REDSHIFT_COREDATAPATH/redshift4maya/2015
export REDSHIFT_SCRIPT_PATH=$REDSHIFT_COREDATAPATH/redshift4maya/common/scripts
export REDSHIFT_XBMLANGPATH=$REDSHIFT_COREDATAPATH/redshift4maya/common/icons
export REDSHIFT_RENDER_DESC_PATH=$REDSHIFT_COREDATAPATH/redshift4maya/common/rendererDesc
export REDSHIFT_CUSTOM_TEMPLATE_PATH=$REDSHIFT_COREDATAPATH/redshift4maya/common/scripts/NETemplates
export REDSHIFT_MAYAEXTENSIONSPATH=$REDSHIFT_PLUG_IN_PATH/extensions
export REDSHIFT_PROCEDURALSPATH=$REDSHIFT_COREDATAPATH/procedurals
export MAYA_PLUG_IN_PATH=$REDSHIFT_PLUG_IN_PATH
export MAYA_SCRIPT_PATH=$REDSHIFT_SCRIPT_PATH
export PYTHONPATH=$REDSHIFT_SCRIPT_PATH
export XBMLANGPATH=$REDSHIFT_XBMLANGPATH
export MAYA_RENDER_DESC_PATH=$REDSHIFT_RENDER_DESC_PATH
export MAYA_CUSTOM_TEMPLATE_PATH=$REDSHIFT_CUSTOM_TEMPLATE_PATH
