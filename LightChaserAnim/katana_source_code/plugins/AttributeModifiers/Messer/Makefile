# Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = Messer.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
PLUGIN_SRC_AMP = $(KATANA_HOME)/plugin_apis/src/FnAttributeModifier

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include
LIBSRC = $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
LIBSRC += $(PLUGIN_SRC_AMP)/plugin/FnAttributeModifier.cpp
LIBSRC += $(PLUGIN_SRC_AMP)/plugin/FnAttributeModifierInput.cpp


CXXFLAGS =	-fPIC -shared $(INCLUDES)
.so.cpp:	$(CC) $(LDFLAGS) $< $@

all:	$(OUTFILEPATH)

$(OUTFILEPATH):	src/Messer.cpp $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ src/Messer.cpp $(LIBSRC)

clean:
	rm $(OUTFILEPATH)
