
# ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.10.0"
ARNOLD_PATH=/home/kt/Develop/arnold-4.2.10.0
OUT=out
INC=src
SRC=src/source
all: shader0
	g++ -o ${OUT}/alTriplanarPlus.so -shared ${OUT}/shader0.os -L${ARNOLD_PATH}/bin -lai
shader0:
	g++ -o ${OUT}/shader0.os -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include -I${INC}/common ${SRC}/alTriplanarPlus.cpp
install:
	@echo "Install ... "
	@cp -f ${OUT}/alTriplanarPlus.so /mnt/work/software/develop/katana/shader/4.x/lcaShaders
	@echo "Done!"
clean:
	rm -f ${OUT}/shader0.os
