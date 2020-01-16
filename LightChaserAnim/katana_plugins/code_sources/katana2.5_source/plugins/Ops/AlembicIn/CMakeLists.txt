add_library(AlembicIn MODULE
    src/ArbitraryGeomParamUtils.cpp
    src/ArrayPropUtils.cpp
    src/CameraCook.cpp
    src/CurvesCook.cpp
    src/FaceSetCook.cpp
    src/NuPatchCook.cpp
    src/ObjectCook.cpp
    src/PointsCook.cpp
    src/PolyMeshCook.cpp
    src/ScalarPropUtils.cpp
    src/SubDCook.cpp
    src/XformCook.cpp
    src/op.cpp)

set_target_properties(AlembicIn PROPERTIES PREFIX "")

target_include_directories(AlembicIn
    PRIVATE
    include)

target_link_libraries(AlembicIn
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::pystring)

target_link_libraries(AlembicIn PRIVATE Alembic::Alembic)

target_include_directories(AlembicIn SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(AlembicIn PRIVATE ${Boost_LIBRARIES})

target_include_directories(AlembicIn SYSTEM PRIVATE ${HDF5_INCLUDE_DIRS})
target_link_libraries(AlembicIn PRIVATE ${HDF5_C_LIBRARIES})

target_link_libraries(AlembicIn PRIVATE OpenEXR::OpenEXR)

install(TARGETS AlembicIn DESTINATION Ops)
