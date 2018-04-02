include(FindPackageHandleStandardArgs)

find_path(PRMAN_INCLUDE_DIR NAMES ri.h PATHS "${RMANTREE}/include")
find_library(PRMAN_LIBRARY NAMES prman libprman PATHS "${RMANTREE}/lib")

set(PRMAN_LIBRARIES "${PRMAN_LIBRARY}")
set(PRMAN_INCLUDE_DIRS "${PRMAN_INCLUDE_DIR}")

find_package_handle_standard_args(PRMan DEFAULT_MSG
   PRMAN_INCLUDE_DIR PRMAN_LIBRARY)

if (NOT TARGET PRMan::PRMan)
    add_library(PRMan::PRMan UNKNOWN IMPORTED)
    set_target_properties(PRMan::PRMan PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PRMAN_INCLUDE_DIR}"
        IMPORTED_LOCATION "${PRMAN_LIBRARY}")
endif ()
