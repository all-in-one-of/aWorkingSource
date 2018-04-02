CXX=g++

EXR_INCLUDES="-I/mnt/work/software/develop/dependences/open_exr/include/OpenEXR"
EXR_LIBDIR="-L/mnt/work/software/develop/dependences/open_exr/lib"

OPT=-o3

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

INCLUDE=$(EXR_INCLUDES)
LIBS=$(EXR_LIBDIR) $(EXRLIBS) -lm 

WARN=-Wall
CXXFLAGS=$(OPT) $(MARCH) $(INCLUDE) $(WARN) $(DEFS)


.PHONY: clean default

default: main

LOGFILE=$(LOGPATH) `date +'%H:%M:%S'`

clean: ; rm -f *.o EXRDataAutoCrop EXRRGBAAutoCrop

main: autocrop_rgba.o autocrop_all.o exr_io.o
	@$(CXX) $(CXXFLAGS) -o EXRRGBAAutoCrop $(LIBS) autocrop_rgba.o exr_io.o
	@$(CXX) $(CXXFLAGS) -o EXRDataAutoCrop $(LIBS) autocrop_all.o exr_io.o
	@echo -e "\033[0;33m --Success! \033[0m"
	@echo [${LOGFILE}] $< "--" $@
autocrop_rgba.o: autocrop_rgba.cpp
	@echo -e "\033[0;33m --Start to comple...! \033[0m"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<
	@echo [${LOGFILE}] $< "--" $@
autocrop_all.o: autocrop_all.cpp
	@echo -e "\033[0;33m --Start to comple...! \033[0m"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<
	@echo [${LOGFILE}] $< "--" $@
exr_io.o: exr_io.cpp
	@$(CXX) $(CXXFLAGS) -o $@ -c $<
	@echo [${LOGFILE}] $< "--" $@