# Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = GeoMakers.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src

INCLUDES = -Iinclude -I$(KATANA_HOME)/plugin_apis/include
LIBSRC = $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
LIBSRC += $(PLUGIN_SRC)/FnScenegraphGenerator/plugin/FnScenegraphGenerator.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp

SOURCES = src/CubeMaker.cpp src/SphereMaker.cpp src/Main.cpp

CXXFLAGS = -fPIC -shared $(INCLUDES)
.so.cpp: $(CC) $< $@

all: $(OUTFILEPATH)

$(OUTFILEPATH): $(SOURCES) $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(LIBSRC)

clean:
	rm $(OUTFILEPATH)
