export ARNOLD_PATH=/mnt/work/software/arnold/arnold-5.0.1.0
INCLUDE=-I${ARNOLD_PATH}/include
LIB=-L${ARNOLD_PATH}/bin
CXXFLAG=-Wall -O2 -shared -fPIC
SRC=src/lc_user_data_float.cpp src/lc_user_data_int.cpp src/lc_user_data_string.cpp src/shader_loader.cpp
OUT=out/osl_user_data.so
all:
	c++ ${SRC} -o ${OUT} ${CXXFLAG} ${INCLUDE} ${LIB} -lai
install:
	cp -f ${OUT} /home/xukai/Git/git_repo/oslShaders