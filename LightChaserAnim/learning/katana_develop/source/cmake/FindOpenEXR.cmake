# FindOpenEXR
# -----------
#
# Try to find the OpenEXR libraries.
#
# Searches following predefined locations:
#
# Use -DOPENEXR_ROOT to provide an alternative search location.
#
# Once done this will define:
#
#  OPENEXR_FOUND - Found OpenEXR
#  OPENEXR_INCLUDE_DIRS - The OpenEXR include directories
#  OPENEXR_LIBRARIES - List containing full path to each of the libraries needed to use OpenEXR.
include(FindPackageHandleStandardArgs)

set(OPENEXR_LIBRARY_SUFFIX "" CACHE STRING "OpenEXR: optional suffix for libraries")
option(OPENEXR_USE_CUSTOM_NAMESPACE "OpenEXR: Set to TRUE to add a set of \
#defines (e.g. -DImath=IMATH_NAMESPACE), such that client code need not be \
aware of the custom OpenEXR namespace." FALSE)

find_path(OPENEXR_INCLUDE_DIR
  NAMES OpenEXR/OpenEXRConfig.h
  HINTS "${OPENEXR_ROOT}"
  PATH_SUFFIXES include)

foreach (component Half Iex Imath IlmImf IlmThread)
    string(TOUPPER "${component}" uppercase_component)
    find_library("OPENEXR_${uppercase_component}_LIBRARY"
        NAMES "${component}${OPENEXR_LIBRARY_SUFFIX}" "${component}"
        HINTS "${OPENEXR_ROOT}"
        PATH_SUFFIXES lib lib64)
endforeach ()

find_package_handle_standard_args(OpenEXR DEFAULT_MSG
    OPENEXR_INCLUDE_DIR
    OPENEXR_HALF_LIBRARY
    OPENEXR_IEX_LIBRARY
    OPENEXR_IMATH_LIBRARY
    OPENEXR_ILMIMF_LIBRARY
    OPENEXR_ILMTHREAD_LIBRARY)

if (OPENEXR_FOUND)
    set(OPENEXR_INCLUDE_DIRS
        "${OPENEXR_INCLUDE_DIR}"
        "${OPENEXR_INCLUDE_DIR}/OpenEXR")
    set(OPENEXR_LIBRARIES
        "${OPENEXR_HALF_LIBRARY}"
        "${OPENEXR_IEX_LIBRARY}"
        "${OPENEXR_IMATH_LIBRARY}"
        "${OPENEXR_ILM_IMF_LIBRARY}"
        "${OPENEXR_ILM_THREAD_LIBRARY}")

    if (NOT TARGET OpenEXR::OpenEXR)
        add_library(OpenEXR::OpenEXR INTERFACE IMPORTED)
        foreach (component Half Iex Imath IlmImf IlmThread)
            string(TOUPPER "${component}" uppercase_component)
            add_library ("OpenEXR::${component}" UNKNOWN IMPORTED)
            set_target_properties("OpenEXR::${component}" PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${OPENEXR_INCLUDE_DIRS}"
                IMPORTED_LOCATION "${OPENEXR_${uppercase_component}_LIBRARY}")
            set_property(TARGET OpenEXR::OpenEXR APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "OpenEXR::${component}")

            if (NOT component STREQUAL Half AND ${OPENEXR_USE_CUSTOM_NAMESPACE})
                set_target_properties("OpenEXR::${component}" PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS
                        "${component}=${uppercase_component}_NAMESPACE")
            endif ()
        endforeach ()
    endif ()
endif ()
