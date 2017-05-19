ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.11.3"
OUT=out
SRC=src
all: shader0
	g++ -o ${OUT}/cryptomatte.so -shared ${OUT}/shader0.os -L${ARNOLD_PATH}/bin -lai
shader0:
	g++ -o ${OUT}/shader0.os -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include -I${SRC}/common -I${ARNOLD_PATH}/cryptomatte ${SRC}/cryptomatte.cpp
install:
	@echo "Install ... "
	@cp -f ${OUT}/cryptomatte.so /mnt/work/software/develop/katana/shader/4.x/lcaShaders
	@echo "Done!"
clean:
	rm -f ${OUT}/shader0.os
