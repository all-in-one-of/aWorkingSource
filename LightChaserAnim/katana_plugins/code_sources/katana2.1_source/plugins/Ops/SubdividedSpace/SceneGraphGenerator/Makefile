
# KATANA_HOME = /data/apps/katana/$(KATANA_VERSION)

PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
PLUGIN_SRC_SGG = $(KATANA_HOME)/plugin_apis/src/FnScenegraphGenerator

INCLUDES = -Iinclude
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

LIBSRC  = $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/plugin/FnAsset.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp
LIBSRC += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp

LIBSRC += $(PLUGIN_SRC_SGG)/plugin/FnScenegraphGenerator.cpp
LIBSRC += $(PLUGIN_SRC)/FnAsset/plugin/FnFileSequence.cpp
LIBSRC += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp

SOURCES      = src/SubdividedSpace.cpp 

CXXFLAGS = -fPIC -shared $(INCLUDES)
.so.cpp: $(CC) $< $@

all: SubdividedSpace.so

SubdividedSpace.so: $(SOURCES) $(LIBSRC)
	$(CXX) $(CXXFLAGS) -o SubdividedSpace.so $(SOURCES) $(LIBSRC)

clean:
	rm SubdividedSpace.so
