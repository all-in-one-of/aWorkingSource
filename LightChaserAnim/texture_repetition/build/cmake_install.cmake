# Install script for directory: /home/xukai/Documents/TestProgramFactory/texture_repetition/alshaders

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/xukai/Documents/TestProgramFactory/texture_repetition/install")
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
   "/home/xukai/Documents/TestProgramFactory/texture_repetition/build/dist/1.0.0rc16/ai/README;/home/xukai/Documents/TestProgramFactory/texture_repetition/build/dist/1.0.0rc16/ai/INSTALL;/home/xukai/Documents/TestProgramFactory/texture_repetition/build/dist/1.0.0rc16/ai/alsUtil.mel")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/xukai/Documents/TestProgramFactory/texture_repetition/build/dist/1.0.0rc16/ai" TYPE FILE FILES
    "/home/xukai/Documents/TestProgramFactory/texture_repetition/alshaders/README"
    "/home/xukai/Documents/TestProgramFactory/texture_repetition/alshaders/INSTALL"
    "/home/xukai/Documents/TestProgramFactory/texture_repetition/alshaders/common/alsUtil.mel"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alSurface/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alRemap/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alCombine/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alNoise/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alLayer/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alPattern/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alCurvature/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alBlackbody/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alInputVector/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alHair/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alColorSpace/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alTriplanar/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alSwitch/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alJitterColor/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alCache/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alCel/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/alFlake/cmake_install.cmake")
  include("/home/xukai/Documents/TestProgramFactory/texture_repetition/build/common/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/xukai/Documents/TestProgramFactory/texture_repetition/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
