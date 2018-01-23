# See README for instructions on how to use this Makefile

# The output objects dir
OBJDIR = ./out
KATANA_HOME=/mnt/work/software/katana/katana2.5v5
INSTALLDIR = /home/xukai/Git/git_repo/katana/resource/2.5/Ops

OUTFILENAME = AttributeSetString.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = src/op.cpp

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnGeolibCookInterfaceUtilsService.cpp
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH = -L$(KATANA_HOME)/bin/Geolib3/internal/CEL
LIBS = -lCEL

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling AttributeSet "
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning AttributeSet"
	@rm -rf $(OBJDIR)

install:
	@echo "  Installing to $(INSTALLDIR)"
	@mkdir -p $(INSTALLDIR)
	@cp $(OUTFILEPATH) $(INSTALLDIR)
