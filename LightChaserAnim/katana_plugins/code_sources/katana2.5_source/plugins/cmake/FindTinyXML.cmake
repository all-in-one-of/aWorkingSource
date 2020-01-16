include(FindPackageHandleStandardArgs)

find_path(TINYXML_INCLUDE_DIR NAMES tinyxml.h)
find_library(TINYXML_LIBRARY NAMES tinyxml)

set(TINYXML_LIBRARIES "${TINYXML_LIBRARY}")
set(TINYXML_INCLUDE_DIRS "${TINYXML_INCLUDE_DIR}")

find_package_handle_standard_args(TinyXML DEFAULT_MSG
    TINYXML_INCLUDE_DIR TINYXML_LIBRARY)

if (NOT TARGET TinyXML::TinyXML)
    add_library(TinyXML::TinyXML UNKNOWN IMPORTED)
    set_target_properties(TinyXML::TinyXML PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${TINYXML_INCLUDE_DIR}"
        IMPORTED_LOCATION "${TINYXML_LIBRARY}")
endif ()
