# See README for instructions on how to use this Makefile

# The output objects dir
OBJDIR = ./out

OUTFILENAME = LocalLocation.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plug-in sources
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src

SOURCES = src/LocalLocation.cpp

# Add plug-in APIs source files
SOURCES += $(PLUGIN_SRC)/pystring/pystring.cpp
SOURCES += $(PLUGIN_SRC)/FnLogging/client/FnLogging.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnAttribute.cpp
SOURCES += $(PLUGIN_SRC)/FnAttribute/client/FnDataBuilder.cpp
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnRenderOutputLocation/plugin/FnRenderOutputLocationPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnDefaultAttributeProducer/plugin/FnDefaultAttributeProducerUtil.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultAssetPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnAsset/client/FnDefaultFileSequencePlugin.cpp

# Include folders
INCLUDES  = -I./include
INCLUDES += -I$(KATANA_HOME)/plugin_apis/include

# Compiler flags
CXXFLAGS=-Wall -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC
LIBS = -lrt

OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling LocalLocation"
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning LocalLocation"
	@rm -rf $(OBJDIR)
