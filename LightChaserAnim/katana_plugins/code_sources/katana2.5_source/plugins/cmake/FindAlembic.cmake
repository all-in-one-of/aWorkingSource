# FindAlembic.cmake
# -----------------

# This module defines:
#
# ALEMBIC_FOUND - Whether Alembic has been found.
#
# If ALEMBIC_FOUND is TRUE, also sets the following:
# ALEMBIC_INCLUDE_DIRS - The Alembic include directories
# ALEMBIC_LIBRARIES - List containing full path to each of the libraries needed
#   to use Alembic.
include(FindPackageHandleStandardArgs)

set(ALEMBIC_LIBRARY_SUFFIX "" CACHE STRING
    "Alembic: optional suffix for library names.")

set(_alembicComponents
    AbcGeom
    Abc
    AbcCoreFactory
    AbcCoreHDF5
    AbcCoreOgawa
    AbcCoreAbstract
    Ogawa
    Util
    AbcCollection
    AbcMaterial)

find_path(ALEMBIC_INCLUDE_DIR
    NAMES Alembic/Abc/All.h
    HINTS "${ALEMBIC_ROOT}"
    PATH_SUFFIXES include)

foreach (component ${_alembicComponents})
    string(TOUPPER "${component}" uppercase_component)
    find_library("ALEMBIC_${uppercase_component}_LIBRARY"
        NAMES "Alembic${component}${ALEMBIC_LIBRARY_SUFFIX}" "Alembic${component}"
        HINTS "${ALEMBIC_ROOT}"
        PATH_SUFFIXES lib lib64)
endforeach ()

find_package_handle_standard_args(Alembic DEFAULT_MSG
    ALEMBIC_INCLUDE_DIR
    ALEMBIC_ABCGEOM_LIBRARY
    ALEMBIC_ABC_LIBRARY
    ALEMBIC_ABCCOREFACTORY_LIBRARY
    ALEMBIC_ABCCOREHDF5_LIBRARY
    ALEMBIC_ABCCOREOGAWA_LIBRARY
    ALEMBIC_ABCCOREABSTRACT_LIBRARY
    ALEMBIC_OGAWA_LIBRARY
    ALEMBIC_UTIL_LIBRARY
    ALEMBIC_ABCCOLLECTION_LIBRARY
    ALEMBIC_ABCMATERIAL_LIBRARY

    # Alembic's transitive dependencies must have been located prior to calling
    # find_package().
    Boost_INCLUDE_DIRS
    HDF5_HL_LIBRARIES
    HDF5_C_LIBRARIES
    HDF5_C_LIBRARIES
    OPENEXR_LIBRARIES
    ZLIB_LIBRARIES)

if (ALEMBIC_FOUND)
    set(ALEMBIC_INCLUDE_DIRS
        "${ALEMBIC_INCLUDE_DIR}"
        "${ALEMBIC_INCLUDE_DIR}/Alembic")

    set(ALEMBIC_LIBRARIES
        ALEMBIC_ABCGEOM_LIBRARY
        ALEMBIC_ABC_LIBRARY
        ALEMBIC_ABCCOREFACTORY_LIBRARY
        ALEMBIC_ABCCOREHDF5_LIBRARY
        ALEMBIC_ABCCOREOGAWA_LIBRARY
        ALEMBIC_ABCCOREABSTRACT_LIBRARY
        ALEMBIC_OGAWA_LIBRARY
        ALEMBIC_UTIL_LIBRARY
        ALEMBIC_ABCCOLLECTION_LIBRARY
        ALEMBIC_ABCMATERIAL_LIBRARY
        ${HDF5_HL_LIBRARIES}
        ${HDF5_C_LIBRARIES}
        ${OPENEXR_LIBRARIES}
        ${ZLIB_LIBRARIES})

    if (NOT TARGET Alembic::Alembic)
        add_library(Alembic::Alembic INTERFACE IMPORTED)
        foreach (component ${_alembicComponents})
            string(TOUPPER "${component}" uppercase_component)

            add_library ("Alembic::${component}" UNKNOWN IMPORTED)
            set_property(TARGET "Alembic::${component}" PROPERTY
                IMPORTED_LOCATION
                    "${ALEMBIC_${uppercase_component}_LIBRARY}")
            set_property(TARGET "Alembic::${component}" APPEND PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES
                    ${ALEMBIC_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS})
            set_property(TARGET "Alembic::${component}" APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES
                    ${HDF5_HL_LIBRARIES}
                    ${HDF5_C_LIBRARIES}
                    ${OPENEXR_LIBRARIES}
                    ${ZLIB_LIBRARIES})

            set_property(TARGET Alembic::Alembic APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "Alembic::${component}")
        endforeach ()
    endif ()
endif ()
