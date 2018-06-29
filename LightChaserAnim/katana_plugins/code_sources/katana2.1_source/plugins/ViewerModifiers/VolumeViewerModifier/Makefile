# Copyright (c) 2011-2013 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = VolumeViewerModifier.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src

INCLUDES = -Iinclude -I$(KATANA_HOME)/plugin_apis/include
INCLUDES += -I$(OPENEXR_HOME)/include

LIBSRC = $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
LIBSRC += $(PLUGIN_SRC)/FnViewerModifier/plugin/FnViewerModifier.cpp
LIBSRC += $(PLUGIN_SRC)/FnViewerModifier/plugin/FnViewerModifierInput.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp

LDFLAGS = -lGL -lGLU

CXXFLAGS =	-fPIC -shared $(INCLUDES)

all:	$(OUTFILEPATH)

$(OUTFILEPATH):	src/VolumeViewerModifier.cpp $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ src/VolumeViewerModifier.cpp $(LIBSRC) $(LDFLAGS)

clean:
	rm $(OUTFILEPATH)
