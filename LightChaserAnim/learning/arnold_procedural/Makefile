OUT_PATH = out
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/proj/software/develop/katana/sources/ArnoldSDKs/Arnold-4.2.10.0-linux

all:
	@echo "Start Compiling..."
	c++ -o ${OUT_PATH}/random_flake.os -c -fPIC -D_LINUX -fPIC -I. -I${ARNOLD_PATH}/include src/random_flake.cpp
	c++ -o ${OUT_PATH}/librandom_flake.so -shared ${OUT_PATH}/random_flake.os -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"