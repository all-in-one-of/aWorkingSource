
INSTALL_DIR=.

CXX=g++

EXR_INCLUDESS="-I/mnt/work/software/develop/dependences/open_exr/include/OpenEXR"
EXR_LIBDIR="-L/mnt/work/software/develop/dependences/open_exr/lib"

OBJDIR = ./out


EXRLIBS=$(EXR_LIBDIR) -Bstatic -lIex -lIlmImf -lIlmThread -lImath -lIex -lHalf -Bdynamic

ARCH = $(shell uname)
ifeq ($(ARCH),Linux)
  EXRLIBS += -lpthread
endif
ifeq ($(ARCH),OpenBSD)
  EXRLIBS += -lpthread
endif
ifeq ($(ARCH),Darwin)
  EXRLIBS += -lz
endif

INCLUDES=$(EXR_INCLUDESS)
LIBS=$(EXR_LIBDIR) $(EXRLIBS) -lm 

WARN=-Wall
OPT=-o3
CXXFLAGS=$(OPT) $(MARCH) $(INCLUDES) $(WARN) $(DEFS)

SRC_DIR := ./src
OBJ_DIR := ./out
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY: clean default install start

default: main

LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`


main: start $(OBJ_FILES)
	@$(CXX) $(CXXFLAGS) -o ${OBJDIR}/OpenEXRScanline $(LIBS) $(OBJ_FILES)
	@echo [${LOGFILE}] "--Finished!"
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [${LOGFILE}] "--Build $< -> $@  "
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

start: ; @echo [${LOGFILE}] "--Start to comple...!"

clean: ; rm -f $(OBJ_FILES) ${OBJDIR}/OpenEXRScanline

install:
	@echo [${LOGFILE}] "--Run the tester..."
	@cp -f ${OBJDIR}/OpenEXRScanline inst/OpenEXRScanline/bin
	@echo [${LOGFILE}] "--Done!"