# Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = SphereMakerMaker.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)
KATANA_HOME=/mnt/work/software/katana/katana2.1v5

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
PLUGIN_SRC_SGG = $(KATANA_HOME)/plugin_apis/src/FnScenegraphGenerator

INCLUDES     = -I$(KATANA_HOME)/plugin_apis/include
LIBSRC = $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC_SGG)/plugin/FnScenegraphGenerator.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp

SOURCES      = src/SphereMakerMaker.cpp

CXXFLAGS = -fPIC -shared $(INCLUDES)
.so.cpp: $(CC) $< $@

all: $(OUTFILEPATH)

$(OUTFILEPATH): $(SOURCES) $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(LIBSRC)
install:
	cp $(OUTFILEPATH) /home/xukai/Git/git_repo/katana/resource/2.1/Libs
clean:
	rm $(OUTFILEPATH)
