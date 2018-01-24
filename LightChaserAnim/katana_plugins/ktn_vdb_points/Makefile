# See README for instructions on how to use this Makefile

# katana2.1v5
KATANA_HOME = /mnt/work/software/katana/katana2.1v5
# katana2.5v5
#~KATANA_HOME = /mnt/work/software/katana/katana2.5v5
INSTALL_TARGET = /home/xukai/Git/git_repo/katana/resource/2.1/Libs

# The output objects dir
OBJDIR = ./out

OUTFILENAME = VdbPoints.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = src/op.cpp

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(shell find $(PLUGIN_SRC)/FnAsset -name \*.cpp)

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC

# Initialize FnAsset API hosts for this Op
CXXFLAGS += -DFNGEOLIBOP_INIT_FNASSET_HOSTS


# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH = -L$(KATANA_HOME)/bin/Geolib3/internal/CEL

LIBS = -lCEL

# Vdb
INCLUDES += -I/mnt/proj/software/lib/OpenVDB/include
LIBPATH += -L/mnt/proj/software/lib/OpenVDB/lib
LIBS += -lopenvdb

HT = /mnt/usr/hfs15/toolkit
HDSO = /mnt/usr/hfs15/dsolib

INCLUDES += -I$(HT)/include
LIBPATH += -L$(HDSO)
LIBS += -lboost_iostreams -lboost_system -lboost_thread

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling VdbPoints "
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning VdbPoints"
	@rm -rf $(OBJDIR)

install:
	@echo " Installing to $(INSTALL_TARGET) "
	@cp $(OUTFILEPATH) $(INSTALL_TARGET)

