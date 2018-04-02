export ARNOLD_PATH=/mnt/work/software/arnold/arnold-5.0.2.1

all:
	c++ src/lc_user_data_gather.cpp -o out/lc_user_data_gather.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	cp -f out/lc_user_data_gather.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders