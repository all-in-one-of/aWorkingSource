# Katana Example Plug-ins

This directory contains a number of example plug-ins for Katana, grouped into
separate CMake projects:

- Asset
- Ops
- RenderOutputLocations
- Templates
- ViewerModifiers

## Building

The plug-ins are built using [CMake](cmake.org). We require version 3.2 or
later. You may be able to install a compatible version through your OS's
package manager; if not, binaries are available at
<https://cmake.org/download/>.

A number of plug-ins require third-party dependencies, such as
[Boost](http://www.boost.org) or [OpenEXR](https://github.com/openexr/openexr).
If these dependencies are not met, the plug-ins that require them will not
build and a warning will be printed.

You can specify the locations of these dependencies using CMake's
[`CMAKE_PREFIX_PATH`](https://cmake.org/cmake/help/v3.2/variable/CMAKE_PREFIX_PATH.html)
configuration variable.

Invoking CMake to build the Op plug-ins might look like this:

### Linux Example
```
# Create a directory in which the build artefacts will be created.
mkdir my-build-tree
cd my-build-tree

# Configure the project
cmake /opt/Foundry/Katana2.5v1/plugins/Src/Ops      \
   -DCMAKE_BUILD_TYPE=Release                       \
   -DCMAKE_INSTALL_PREFIX="$HOME/MyKatanaResources" \
   -DKATANA_HOME=/opt/Foundry/Katana2.5v1           \
   -DCMAKE_PREFIX_PATH=/sw/alembic150;/sw/openexr22;/sw/boost155

# Build and install the project
cmake --build .
cmake --build . --target install

# Build only a single plug-in
cmake --build . --target Prune
```

### Windows Example
```
rem Create a directory in which the build artefacts will be created.
mkdir my-build-tree
cd my-build-tree

rem Configure the project.
cmake "C:\Program Files\Katana2.5v1\plugins\Src\Ops"        ^
   -G"Visual Studio 10 2010 Win64"                          ^
   "-DCMAKE_INSTALL_PREFIX=%USERPROFILE%\MyKatanaResources" ^
   "-DKATANA_HOME=C:\Program Files\Katana2.5dev"            ^
   -DCMAKE_PREFIX_PATH=C:\sw\openexr22;c:\sw\alembic150;C:\sw\boost155;C:\sw\hdf5;C:\sw\zlib

rem Build and install the project
cmake --build . --config Release
cmake --build . --config Release --target install

rem Build only a single plug-in
cmake --build . --config Release --target Prune
```

## CMake Configuration Options

The following CMake command line options may be used to control the build.
Modify the example values for your OS and directory layout as needed:

* `-G<generator name>`

  Specifies the project generator to use. On Linux platforms you need not
  specify this flage generator, as the default of "Unix Makefiles" is generally
  sufficient. On Windows, we have tested with -G"Visual Studio 10 2010 Win64".
  If you are using a more modern version of Visual Studio, change this value as
  appropriate.

  See the CMake documentation for this parameter:
  <https://cmake.org/cmake/help/v3.2/manual/cmake.1.html>. Also see
  `cmake --help` for a list of possible generators.

* `-DCMAKE_BUILD_TYPE=<Debug|Release>`

  Defines the build tree configuration type: Debug or Release. CMAKE_BUILD_TYPE
  is used by most, but not all, generator types.

  Notably, CMake's Visual Studio generator will produce a single build tree
  that supports building both build variants, Debug and Release. With such
  generators, you can configure which variant is built by passing a `--config
  <Release|Debug>` build flag.

* `-DCMAKE_INSTALL_PREFIX=<install prefix>`

  CMake will install example plug-ins to "Ops", "Libs" and "Plugins"
  subdirectories of this path. In order for Katana to load the plug-ins on
  startup, this will need to be added to the KATANA_RESOURCES environment
  variable. See the User Guide for further information.

* `-DALEMBIC_LIBRARY_SUFFIX=<string>`

  Required if your Alembic libraries have a custom suffix (e.g.
  "-MyFacilityName").

* `-DBoost_USE_STATIC_LIBS=<TRUE|FALSE>`

  Set to `TRUE` if you have a statically-linked version of Boost. This flag may
  be required on Windows.

* `-DOPENEXR_LIBRARY_SUFFIX=<string>`

  Required if your OpenEXR libraries have a custom suffix (e.g.
  "-MyFacilityName").

* `-DOPENEXR_USE_CUSTOM_NAMESPACE=<TRUE|FALSE>`

  Set to `TRUE` if your OpenEXR libraries have been compiled with a custom C++
  namespace.
