# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = SphereRender.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)

# Plugin source dirs
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
PLUGIN_SRC_VMP = $(KATANA_HOME)/plugin_apis/src/FnViewerModifier

INCLUDES     = -I$(KATANA_HOME)/plugin_apis/include

# PLUGIN APIs source
LIBSRC = $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
LIBSRC += $(PLUGIN_SRC)/FnViewerModifier/plugin/FnViewerModifier.cpp
LIBSRC += $(PLUGIN_SRC)/FnViewerModifier/plugin/FnViewerModifierInput.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
LIBSRC += -lGL -lGLU

# Libs
LDFLAGS = -lGL -lGLU

SOURCES      = src/SphereRender.cpp

CXXFLAGS = -fPIC -shared $(INCLUDES)
.so.cpp: $(CC) $< $@

all: $(OUTFILEPATH)

$(OUTFILEPATH): $(SOURCES) $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(LIBSRC) $(LDFLAGS)

clean:
	rm $(OUTFILEPATH)
