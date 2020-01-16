#!/usr/bin/env python

import os
from optparse import OptionParser
from string import Template
from datetime import date

#------------------------------------------------------------------------------

def _parseOptions():
  usage = "usage: %prog [options] OpName1 ... OpNameN"

  parser = OptionParser(usage)

  # Configure the option parser
  parser.add_option("-d", "--directory",
                    dest = "directory",
                    action = "store",
                    default = os.path.abspath(os.path.curdir),
                    help = "Directory to create the new Op in, if not " +\
                          "specified current directory will be used.",
                    metavar = "DIR")

  parser.add_option("-i", "--include-hello",
                    dest = "hello_world",
                    action = "store_true",
                    default = False,
                    help = "Include a small 'Hello World' Op example in " +\
                           "the boiler plate op.cpp file")

  parser.add_option("-a", "--no-assetapi-init",
                    dest = "init_assetapi",
                    action = "store_false",
                    default = True,
                    help = "Don't add compiler flags and code to " + \
                           "initialize FnAsset API for this Op")

  (options, opNames) = parser.parse_args()

  # Quick sanity check of the options we've been passed
  if len(opNames) < 1:
    print("You must specify at least one Op name to create")
    exit(-1)
  else:
    return (options, opNames)

#------------------------------------------------------------------------------

def _createFile(absolutePath, contents):
  f = open(absolutePath, "w")
  f.write(contents)
  f.close()

#------------------------------------------------------------------------------

def _createOps(options, opNames):

  for op in opNames:
    # For each op we need to:
    # 1). Check if the directory exists if yes skip else create it
    # 2). Create the 3 files README src/op.cpp and Makefile
    # 3). op.cpp should be filled according to options.hello_world
    opRootDir = os.path.join(options.directory, op)

    if os.path.exists(opRootDir):
      print("%s already exists in %s so skipping" % (op, opRootDir))
      continue

    # Create the directory structure
    opSrcDir = os.path.join(opRootDir, "src")
    try:
      os.makedirs(opRootDir)
      os.makedirs(opSrcDir)
    except IOError, exception:
      print("Could not create Op folder structure for %s error %s"
          % (op, exception.strerror))
    except Exception, exception:
      print("Could not create Op folder structure for %s error %s"
          % (op, exception))
      continue

    # Build strings for AssetAPI initialization
    assetAPIIncludes = ""
    assetAPICxxFlags = ""
    assetAPISources = ""

    if options.init_assetapi:
      assetAPIIncludes = _ASSETAPI_INCLUDES
      assetAPICxxFlags = _ASSETAPI_CXXFLAGS
      assetAPISources = _ASSETAPI_SOURCES

    # Create the README
    readmePath = os.path.join(opRootDir, "README")
    _createFile(readmePath, _README)

    # Create the Makefile
    tmplt = Template(_MAKEFILE_TEMPLATE)
    contents = tmplt.substitute(OpName = op,
                                AssetAPISources = assetAPISources,
                                AssetAPICxxFlags = assetAPICxxFlags)
    makefilePath = os.path.join(opRootDir, "Makefile")
    _createFile(makefilePath, contents)

    # Create the src/op.cpp
    tmplt = Template(_SRCFILE_TEMPLATE)
    contents = tmplt.substitute(OpName = op,
                                Name = os.environ["USER"],
                                Date = date.today().isoformat(),
                                AssetAPIIncludes = assetAPIIncludes,
                                BoilerPlateCode = options.hello_world and _BOILERPLATE_HELLOWORLD or _BOILERPLATE_COMMENT
                                )

    srcfilePath = os.path.join(opSrcDir, "op.cpp")
    _createFile(srcfilePath, contents)

    print("Created %s" % (op))

#------------------------------------------------------------------------------

def main():
  (options, opNames) = _parseOptions()

  _createOps(options, opNames)

  exit(0)

_README = """README
Instructions
1). Set your KATANA_HOME environment variable to point to your Geolib3
Katana installation
2). Run:
  make && make install
"""

_ASSETAPI_INCLUDES = """
// FnDefaultAssetPlugin and FnDefaultFileSequencePlugin initialization is
// currently active.
// The FNGEOLIBOP_INIT_FNASSET_HOSTS flag will be also passed to the compiler
// when building this Op.
//
// Notice that initializing FnDefaultAsset and FnDefaultFileSequence plug-ins
// will increase the library size and the compile time.
//
// Use 'CreateOp -a' or CreateOp --no-assetapi-init' to avoid adding compiler
// flags and code to initialize FnAsset API for an Op.
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>
"""

_SRCFILE_TEMPLATE = """// Description: <Description of your Op>
// Author: ${Name}
// Date: ${Date}

#include <iostream>
#include <string>

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
${AssetAPIIncludes}

namespace { //anonymous

class ${OpName}Op : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        ${BoilerPlateCode}
    }
};

DEFINE_GEOLIBOP_PLUGIN(${OpName}Op)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(${OpName}Op, "${OpName}", 0, 1);
}

"""

_BOILERPLATE_COMMENT = """
        /* Insert your Op code here */
"""

_BOILERPLATE_HELLOWORLD = """
        FnGeolibOp::CreateLocationInfo locationInfo;
        FnGeolibOp::CreateLocation(locationInfo, interface, "/root/world/hello");

        // Set some attributes
        if(interface.getOutputLocationPath() == "/root/world")
        {
            interface.setAttr("at_world", FnAttribute::StringAttribute("at_world"));
        }

        if(interface.getOutputLocationPath() == "/root/world/hello")
        {
            interface.setAttr("at_hello", FnAttribute::StringAttribute("at_hello"));
        }
"""

_ASSETAPI_SOURCES = "SOURCES += $(shell find $(PLUGIN_SRC)/FnAsset -name \*.cpp)"

_ASSETAPI_CXXFLAGS = """
# Initialize FnAsset API hosts for this Op
CXXFLAGS += -DFNGEOLIBOP_INIT_FNASSET_HOSTS
"""

_MAKEFILE_TEMPLATE = """# See README for instructions on how to use this Makefile

# The output objects dir
OBJDIR = ./out

OUTFILENAME = ${OpName}.so
OUTFILEPATH = $$(OBJDIR)/$$(OUTFILENAME)

# Plugin sources and includes
PLUGIN_SRC = $$(KATANA_HOME)/plugin_apis/src
SOURCES = src/op.cpp

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $$(shell find $$(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $$(shell find $$(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $$(shell find $$(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $$(shell find $$(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
SOURCES += $$(shell find $$(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $$(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
${AssetAPISources}

INCLUDES = -I$$(KATANA_HOME)/plugin_apis/include

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC
${AssetAPICxxFlags}

# Object files and flags
OBJS := $$(patsubst %.cpp,$$(OBJDIR)/%.o,$$(SOURCES))

LIBPATH = -L$$(KATANA_HOME)/bin/Geolib3/internal/CEL
LIBS = -lCEL

# Targets:
all: $$(OUTFILEPATH)

$$(OUTFILEPATH): $$(OBJS)
	@echo "  Compiling ${OpName} "
	$$(CXX) $$(CXXFLAGS) $$(OBJS) $$(LIBPATH) $$(LIBS) -shared -o $$(OUTFILEPATH) -Wl,-soname,$$(OUTFILENAME)

$$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $$@`
	$$(CXX) $$(CXXFLAGS) $$(INCLUDES) -c $$< -o $$@

clean:
	@echo "  Cleaning ${OpName}"
	@rm -rf $$(OBJDIR)

install:
	@echo " Installing to $$(KATANA_HOME)/bin/Geolib3/Ops "
	@cp $$(OUTFILEPATH) $$(KATANA_HOME)/bin/Geolib3/Ops

"""
if __name__ == "__main__":
  main()

