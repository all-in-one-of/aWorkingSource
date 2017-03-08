# Install script for directory: /home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RELEASE")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/dist/1.0.0rc19/ai/README;/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/dist/1.0.0rc19/ai/INSTALL;/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/dist/1.0.0rc19/ai/alsUtil.mel")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/dist/1.0.0rc19/ai" TYPE FILE FILES
    "/home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders/README"
    "/home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders/INSTALL"
    "/home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders/common/alsUtil.mel"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alSurface/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alRemap/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alCombine/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alNoise/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alLayer/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alPattern/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alCurvature/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alBlackbody/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alInputVector/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alHair/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alColorSpace/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alTriplanar/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alSwitch/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alJitterColor/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alCache/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alCel/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/alFlake/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/cryptomatte/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/common/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build_maya/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
