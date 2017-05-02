add_library(FileAsset MODULE src/FileAsset.cpp)

target_include_directories(FileAsset PRIVATE include)

target_link_libraries(FileAsset
    PRIVATE
    Katana::FnAsset
    Katana::FnAssetPlugin
    Katana::FnAttribute
    Katana::FnLogging
    Katana::pystring
)

set_target_properties(FileAsset PROPERTIES PREFIX "")

target_include_directories(FileAsset PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(FileAsset PRIVATE ${Boost_LIBRARIES})

install(TARGETS FileAsset DESTINATION Libs)
