# Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
# See README for instructions on how to use this Makefile
#

OUTDIR = ./out
OUTFILENAME = ScenegraphXML.so
OUTFILEPATH = $(OUTDIR)/$(OUTFILENAME)

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
PLUGIN_SRC_SGG = $(KATANA_HOME)/plugin_apis/src/FnScenegraphGenerator

INCLUDES = -Iinclude
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

# Boost
BOOST_INCLUDE ?= $(BOOST_HOME)/include
INCLUDES += -I$(BOOST_INCLUDE)
LIBPATH   =  -L$(BOOST_HOME)/lib
LIBS      =  -lboost_filesystem -lboost_system

LIBSRC =  $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC_SGG)/plugin/FnScenegraphGenerator.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/plugin/FnAsset.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/plugin/FnFileSequence.cpp

SOURCES =  src/ArbitraryList.cpp
SOURCES += src/Bounds.cpp
SOURCES += src/Channels.cpp
SOURCES += src/ErrorReport.cpp
SOURCES += src/instance_scene_in.cpp
SOURCES += src/Instance.cpp
SOURCES += src/LodData.cpp
SOURCES += src/Modifier.cpp
SOURCES += src/ProceduralArgument.cpp
SOURCES += src/ProxyList.cpp
SOURCES += src/Scene.cpp
SOURCES += src/ScenegraphXml_In.cpp
SOURCES += src/Xform.cpp

CXXFLAGS = -fPIC -shared $(INCLUDES)
.so.cpp: $(CC) $< $@

all: $(OUTFILEPATH)

$(OUTFILEPATH): $(SOURCES) $(LIBSRC)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(LIBSRC) $(LIBPATH) $(LIBS)

clean:
	rm $(OUTFILEPATH)
