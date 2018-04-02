export ARNOLD_PATH=/mnt/work/software/arnold/arnold-5.0.2.1
SOURCE_CODE=src/lc_user_data_float.cpp
SOURCE_CODE +=src/lc_user_data_int.cpp
SOURCE_CODE +=src/shader_loader.cpp

all:
	c++ ${SOURCE_CODE} -o out/lc_user_data.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	cp -f out/lc_user_data.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders