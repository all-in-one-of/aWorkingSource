
ARNOLD_PATH=/mnt/work/software/develop/arnold-SDK/arnold-5.0.1.0
OUT=./out
SRC=./src
CXXFLAGS=-Wall -O2 -shared -fPIC
INCLUDES=-I${ARNOLD_PATH}/include
LIBS=-L${ARNOLD_PATH}/bin -lai
all:
	c++ ${SRC}/lc_outline_filter.cpp -o ${OUT}/lc_outline_filter.so ${CXXFLAGS} ${INCLUDES} ${LIBS}
install:
	cp -f ${OUT}/lc_outline_filter.so /home/xukai/Git/git_lab/lcaShaders
	@echo "DONE!"