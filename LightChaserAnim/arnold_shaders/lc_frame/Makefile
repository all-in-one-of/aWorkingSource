ARNOLD_PATH="/mnt/work/software/develop/arnold-SDK/arnold-5.0.1.1"

all:
	@echo "Compile..."
	c++ src/lc_frame.cpp -o out/lc_frame.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
	@echo "Finish!"
	@echo ""
install:
	@echo "Install..."
	cp -f out/lc_frame.so /home/xukai/Git/git_lab/lcaShaders
	cp -f src/lc_frame.mtd /home/xukai/Git/git_lab/lcaShaders
	@echo "--- Job Done! ---!"
