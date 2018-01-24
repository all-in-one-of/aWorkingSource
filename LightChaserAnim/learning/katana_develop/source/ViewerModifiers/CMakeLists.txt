cmake_minimum_required (VERSION 3.2)
project(KatanaViewerModifierPlugins)

include(CheckSymbolExists)
include(CMakePushCheckState)

set(KATANA_HOME "${CMAKE_CURRENT_LIST_DIR}/../../.." CACHE PATH "Path to Katana")
list(INSERT CMAKE_MODULE_PATH 0 "${KATANA_HOME}/plugins/Src/cmake")

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
    message(WARNING "Not compiling ViewerModifier plugins as OpenGL was not found.")
    return ()
endif ()

if (GLEW_FOUND)
    # Found GLEW, but check it is a recent enough version.
    cmake_push_check_state()
    set(CMAKE_REQUIRED_INCLUDES "${GLEW_INCLUDE_DIRS}")
    check_symbol_exists(glMultiDrawElementsIndirect
        "GL/glew.h" HAVE_DECL_GLMULTIDRAWELEMENTSINDIRECT)
    cmake_pop_check_state()
endif ()

if (OPENEXR_FOUND AND GLEW_FOUND AND HAVE_DECL_GLMULTIDRAWELEMENTSINDIRECT)
    add_subdirectory(BatchViewerModifier)
else ()
    message(WARNING "Not compiling BatchViewerModifier plugin as not all dependencies were found.")
endif ()
if (OPENEXR_FOUND)
    add_subdirectory(VolumeViewerModifier)
else ()
    message(WARNING "Not compiling VolumeViewerModifier plugin as OpenEXR was not found.")
endif ()

add_subdirectory(CameraViewerModifier)
add_subdirectory(LightFilterViewerModifier)
add_subdirectory(LightViewerModifier)
add_subdirectory(SphereRender)
