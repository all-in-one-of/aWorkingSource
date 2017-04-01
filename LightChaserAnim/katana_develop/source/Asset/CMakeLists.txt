cmake_minimum_required (VERSION 3.2)
project(KatanaAssetPlugins)

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

if (MSVC)
    # Disallow Boost auto-linking.
    add_definitions(-DBOOST_ALL_NO_LIB)
endif ()

find_package(Boost 1.55.0 COMPONENTS regex)

if (Boost_FOUND)
    add_subdirectory(FileAsset)
else ()
    message(WARNING "Not compiling FileAsset plugin -- could not find Boost.")
endif ()
