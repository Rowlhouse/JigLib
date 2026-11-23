# Builds the JigLib physics library and, if the JigTest folder is present, the JigTest demo.
#
# JigLib is self-contained: to use it in your own application, add the JigLib folder to your
# project with add_subdirectory() and link to JigLib::jiglib. JigTest is just one such application.

cmake_minimum_required(VERSION 3.21)

project(JigLibAndJigTest LANGUAGES CXX)

# Default to a Release build for single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(JIGLIB_BUILD_TESTS "Build the JigLib smoke test (run with ctest)" ON)
if(JIGLIB_BUILD_TESTS)
  enable_testing()
endif()

add_subdirectory(JigLib)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JigTest/CMakeLists.txt)
  option(JIGLIB_BUILD_JIGTEST "Build the JigTest demo (needs OpenGL; downloads SDL3 etc if not installed)" ON)
  if(JIGLIB_BUILD_JIGTEST)
    add_subdirectory(JigTest)
  endif()
endif()
