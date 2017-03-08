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
   "/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/dist/1.0.0rc19/ai/README;/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/dist/1.0.0rc19/ai/INSTALL;/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/dist/1.0.0rc19/ai/alsUtil.mel")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/dist/1.0.0rc19/ai" TYPE FILE FILES
    "/home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders/README"
    "/home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders/INSTALL"
    "/home/xukai/Documents/TestProgramFactory/cryptomatte/alshaders/common/alsUtil.mel"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alSurface/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alRemap/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alCombine/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alNoise/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alLayer/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alPattern/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alCurvature/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alBlackbody/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alInputVector/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alHair/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alColorSpace/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alTriplanar/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alSwitch/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alJitterColor/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alCache/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alCel/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/alFlake/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/cryptomatte/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/common/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/xukai/Documents/TestProgramFactory/cryptomatte/alShaders_build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
