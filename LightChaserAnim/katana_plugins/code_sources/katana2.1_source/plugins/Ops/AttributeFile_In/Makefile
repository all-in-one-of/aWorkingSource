# See README for instructions on how to use this Makefile

# The output objects dir
OBJDIR = ./out
INSTALLDIR = $(KATANA_HOME)/plugins/Resources/Examples/Ops

OUTFILENAME = AttributeFile_In.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plugin sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = src/op.cpp src/FileReaderFactory.cpp src/KatanaAttrFileReader.cpp

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnGeolibCookInterfaceUtilsService.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp

INCLUDES  = -I$(KATANA_HOME)/plugin_apis/include
INCLUDES += -Iinclude

# TinyXML includes and source
INCLUDES += -I$(TINYXML_HOME)/include
SOURCES  += $(shell find $(TINYXML_HOME)/src -name \*.cpp)

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC

CXXFLAGS += -DTIXML_USE_STL

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH = -L$(KATANA_HOME)/bin/Geolib3/internal/CEL
LIBS = -lCEL

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling AttributeFile_In "
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning AttributeFile_In"
	@rm -rf $(OBJDIR)

install:
	@echo "  Installing to $(INSTALLDIR)"
	@mkdir -p $(INSTALLDIR)
	@cp $(OUTFILEPATH) $(INSTALLDIR)
