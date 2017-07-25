ARNOLD_PATH="/mnt/work/software/arnold/arnold-5.0.1.0"
OUT=out
SRC=src
all:
	c++ ${SRC}/lc_fuse_normal.cpp -o ${OUT}/lc_fuse_normal.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	cp ${OUT}/lc_fuse_normal.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders