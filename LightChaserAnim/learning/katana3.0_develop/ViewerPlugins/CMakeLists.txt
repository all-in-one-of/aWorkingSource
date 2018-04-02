cmake_minimum_required (VERSION 3.2)
project(KatanaViewerPlugins)

include(CheckSymbolExists)
include(CMakePushCheckState)

set(KATANA_HOME "${Katana_SDK_ROOT}" CACHE PATH "Path to Katana")
list(INSERT CMAKE_MODULE_PATH 0 "${KATANA_HOME}/plugins/Src/cmake")

set(CMAKE_CXX_STANDARD 11)

find_package(Katana PATHS "${KATANA_HOME}/plugin_apis/cmake" REQUIRED)

if (NOT MSVC)
    add_compile_options(-Wall)
endif ()

# Ensure we don't compile DSOs with undefined symbols.
if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
endif ()

find_package(GLEW)
find_package(OpenEXR)
find_package(OpenGL)

if (NOT OPENGL_FOUND)
    message(WARNING "Not compiling built-in viewer plugins as OpenGL was not found.")
    return ()
endif ()

if (NOT GLEW_FOUND)
    message(WARNING "Not compiling built-in viewer plugins plugins as GLEW was not found.")
    return ()
endif ()

if (NOT OPENEXR_FOUND)
    message(WARNING "Not compiling built-in viewer plugins plugin as OpenEXR was not found.")
    return ()
endif ()

add_subdirectory(BallLocationType)
add_subdirectory(TransformManipulators)
