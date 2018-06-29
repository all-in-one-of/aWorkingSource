ARNOLD_PATH="/mnt/work/software/arnold/arnold-5.0.2.1"
NAME="lc_volume_noise"
all:
	c++ src/${NAME}.cpp -o out/${NAME}.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	@echo "Install ... "
	cp -f out/${NAME}.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	@echo "Done!"
