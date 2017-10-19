ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.11.3"
INSTALL_PATH="/home/xukai/Git/git_repo/katana/resource/2.1/Libs"
OUT=out
SRC=src

all:
	c++ ${SRC}/driver_json.cpp -o ${OUT}/lc_driver_json.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
test:
	/mnt/work/software/arnold/arnold-4.2.11.3/bin/kick ./ref/example.ass -o ohyeah_output.txt -l ./out
install:
	cp ${OUT}/lc_driver_json.so /home/xukai/Git/git_repo/katana/shader/4.x/devShaders