ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.11.3"
INSTALL_PATH="/home/xukai/Git/git_repo/katana/resource/2.1/Libs"
OUT=out
SRC=src

all:
	c++ ${SRC}/driver_json.cpp -o ${OUT}/lc_json_driver.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
tst:
	/mnt/work/software/arnold/arnold-4.2.11.3/bin/kick /home/xukai/Documents/TestProgramFactory/arnold_json_driver/ref/example.ass -o tst.jpg -l /home/xukai/Documents/TestProgramFactory/arnold_json_driver/out
install:
	cp ${OUT}/lc_json_driver.so /home/xukai/Git/git_repo/katana/shader/4.x/devShaders

# all: tempDriver
# 	c++ -o ${OUT}/JsonDriver.so -shared ${OUT}/tempDriver.os -L${ARNOLD_PATH}/bin -lai
# tempDriver:
# 	c++ -o ${OUT}/tempDriver.os -fPIC -D_LINUX -Wall -Wsign-compare -fvisibility=hidden -fPIC -I${ARNOLD_PATH}/include ${SRC}/driver_json.cpp

#/home/xukai/Git/git_repo/katana/arnold/ktoa-1.0.7-lca/bin/kick -i /home/xukai/Documents/TestProgramFactory/arnold_json_driver/ref/example.ass  -r 640 480 -g 2.2 -o /home/xukai/teapot.jpg
