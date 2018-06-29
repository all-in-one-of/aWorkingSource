
PREFIX_PATH=/home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders

ARNOLD_PATH=/mnt/work/software/arnold/arnold-5.0.2.1
NAME=lc_triplanar
INCLUDES=-I${ARNOLD_PATH}/include
LIBS=-L${ARNOLD_PATH}/bin -lai
FLAGS=-Wall -O2 -shared -fPIC

.PHONY: clean default
LOGFILE=`date +'%H:%M:%S'`

default: main

main:
	@echo -e "[${LOGFILE}] \033[0;33m --Compiling ${NAME} \033[0m"
	@c++ src/${NAME}.cpp -o out/${NAME}.so ${FLAGS} ${INCLUDES} ${LIBS}
	@echo -e "[${LOGFILE}] \033[0;32m --Successed! \033[0m"
install:
	@echo -e "[${LOGFILE}] \033[0;33m --Installing ${NAME} to ${PREFIX_PATH}\033[0m"
	@cp -f out/${NAME}.so ${PREFIX_PATH}
	@cp -f src/${NAME}.mtd ${PREFIX_PATH}
	@echo -e "[${LOGFILE}] \033[0;32m --Successed! \033[0m"
clean: ; @rm -f out/${NAME}.so
	