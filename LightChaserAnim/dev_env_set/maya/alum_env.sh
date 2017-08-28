# ------alum
export LCA_DEVUSD_PATH=/home/xukai/Git/git_repo/usd
export ALUM_ROOT=${LCA_DEVUSD_PATH}/lca_usd_house/alum_centos_v0231
export MAYA_PLUG_IN_PATH=${ALUM_ROOT}/plugin:$MAYA_PLUG_IN_PATH
export PXR_PLUGINPATH=${ALUM_ROOT}/lib:$PXR_PLUGINPATH
export PYTHONPATH=${ALUM_ROOT}/lib/python:$PYTHONPATH
export LD_LIBRARY_PATH==${ALUM_ROOT}/lib:$LD_LIBRARY_PATH