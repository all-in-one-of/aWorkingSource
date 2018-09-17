ARNOLD_PATH=/mnt/work/software/arnold/arnold-5.0.1.0
all:
	c++ src/MayaColorBalance.cpp -o out/MayaColorBalance.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	cp -f out/MayaColorBalance.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	@echo "Done!"