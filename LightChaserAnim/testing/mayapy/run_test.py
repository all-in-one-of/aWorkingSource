import os
ROOT_PATH=os.path.dirname(os.path.realpath(__file__))
MAYA_BIN_PATH="/home/xukai/Git/git_repo/lca_rez/launchers/pws/linux/mayapy"
PYTHON_FILE_NAME=os.path.join(ROOT_PATH,"test.py")
MAYA_FILE_PATH="${ROOT_PATH}/test.ma"
OUTPUT_FILE_PATH="${ROOT_PATH}/test.txt"
PY_ARGS="-s asfdasf -u asfdsgfsdgdfg -n fregrfgrf"


command = MAYA_BIN_PATH+' '+PYTHON_FILE_NAME
os.system(command)