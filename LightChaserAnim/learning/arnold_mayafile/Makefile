OUT_PATH = ./
SOURCE_DIR = src/MayaFile.cpp
SHARED_DIR = ./singalMayaFile.so
#SOURCE_DIR += src/MayaUtils.cpp
#SOURCE_DIR += src/MayaUtils.h
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.11.3-linux

all:
	g++ -o ${OUTPATH}MayaFile.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SOURCE_DIR}

n1:
	g++ -o ${OUTPATH}singalMayaFile.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include ${SOURCE_DIR}
n2:
	g++ -o ${OUTPATH}alMayaFile.so -shared ${SHARED_DIR} -L${ARNOLD_PATH}/bin -lai
install:
	cp -f alMayaFile.so /mnt/proj/software/develop/katana/utility/shaders/devShaders
	@echo "Install alMayaFile ... "
.PHONY : alMayaFile_install
