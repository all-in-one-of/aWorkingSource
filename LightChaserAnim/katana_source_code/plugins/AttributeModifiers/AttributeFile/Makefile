# Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = AttributeFile.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
PLUGIN_SRC_AMP = $(KATANA_HOME)/plugin_apis/src/FnAttributeModifier

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -Iinclude -I$(BOOST_INCLUDE)

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

SOURCE_FILES = src/AttributeFile.cpp src/FileReaderFactory.cpp src/KatanaAttrFileReader.cpp

all:	$(OUTFILEPATH)

$(OUTFILEPATH):	$(SOURCE_FILES) $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCE_FILES) $(LIBSRC)

clean:
	rm $(OUTFILEPATH)
