cmake_minimum_required(VERSION 3.21)

project(JigLib
  VERSION 0.84
  DESCRIPTION "A rigid body physics library"
  LANGUAGES CXX)

option(JIGLIB_USE_DOUBLE "Use double precision for tScalar" OFF)
option(JIGLIB_WARNINGS_AS_ERRORS "Treat compiler warnings as errors (used by CI)" OFF)

# Default to a Release build for single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(JIGLIB_COMPONENTS collision geometry maths physics utils vehicles)

set(JIGLIB_SOURCES
  collision/src/colldetectboxbox.cpp
  collision/src/colldetectboxheightmap.cpp
  collision/src/colldetectboxplane.cpp
  collision/src/colldetectboxstaticmesh.cpp
  collision/src/colldetectcapsulebox.cpp
  collision/src/colldetectcapsulecapsule.cpp
  collision/src/colldetectcapsuleheightmap.cpp
  collision/src/colldetectcapsuleplane.cpp
  collision/src/colldetectcapsulestaticmesh.cpp
  collision/src/colldetectspherebox.cpp
  collision/src/colldetectspherecapsule.cpp
  collision/src/colldetectsphereheightmap.cpp
  collision/src/colldetectsphereplane.cpp
  collision/src/colldetectspheresphere.cpp
  collision/src/colldetectspherestaticmesh.cpp
  collision/src/collisioninfo.cpp
  collision/src/collisionskin.cpp
  collision/src/collisionsystem.cpp
  collision/src/collisionsystembrute.cpp
  collision/src/collisionsystemgrid.cpp
  collision/src/materials.cpp
  geometry/src/aabox.cpp
  geometry/src/box.cpp
  geometry/src/capsule.cpp
  geometry/src/distance.cpp
  geometry/src/heightmap.cpp
  geometry/src/indexedtriangle.cpp
  geometry/src/intersection.cpp
  geometry/src/octree.cpp
  geometry/src/plane.cpp
  geometry/src/primitive.cpp
  geometry/src/sphere.cpp
  geometry/src/trianglemesh.cpp
  maths/src/matrix33.cpp
  maths/src/vector3.cpp
  physics/src/body.cpp
  physics/src/constraint.cpp
  physics/src/constraintmaxdistance.cpp
  physics/src/constraintpoint.cpp
  physics/src/constraintvelocity.cpp
  physics/src/constraintworldpoint.cpp
  physics/src/hingejoint.cpp
  physics/src/physicscontroller.cpp
  physics/src/physicssystem.cpp
  utils/src/assert.cpp
  utils/src/configfile.cpp
  utils/src/timer.cpp
  utils/src/trace.cpp
  vehicles/src/car.cpp
  vehicles/src/chassis.cpp
  vehicles/src/wheel.cpp
)

# Headers are listed so that they show up in IDEs
file(GLOB JIGLIB_HEADERS CONFIGURE_DEPENDS
  include/*.hpp
  */include/*.hpp
  */include/*.inl
  physics/src/*.hpp)

add_library(jiglib STATIC ${JIGLIB_SOURCES} ${JIGLIB_HEADERS})
add_library(JigLib::jiglib ALIAS jiglib)

target_compile_features(jiglib PUBLIC cxx_std_17)

# The headers include each other both by plain name and by paths such as
# "../maths/include/vector3.hpp", which resolve relative to include/.
target_include_directories(jiglib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/collision/include
  ${CMAKE_CURRENT_SOURCE_DIR}/geometry/include
  ${CMAKE_CURRENT_SOURCE_DIR}/maths/include
  ${CMAKE_CURRENT_SOURCE_DIR}/physics/include
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vehicles/include)

# These affect inline code in the headers, so they are public
target_compile_definitions(jiglib PUBLIC
  USE_FUNCTION
  $<$<CONFIG:Debug>:DEBUG _DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:RELEASE>
  $<$<BOOL:${JIGLIB_USE_DOUBLE}>:USING_DOUBLE>)

if(MSVC)
  target_compile_definitions(jiglib PRIVATE _CRT_SECURE_NO_WARNINGS)
  # C4267: size_t to unsigned conversions. Container sizes are stored as unsigned throughout, and
  # never get anywhere near 2^32.
  target_compile_options(jiglib PRIVATE /W3 /wd4267 /permissive-)
  if(JIGLIB_WARNINGS_AS_ERRORS)
    target_compile_options(jiglib PRIVATE /WX)
  endif()
else()
  target_compile_options(jiglib PRIVATE -Wall)
  if(JIGLIB_WARNINGS_AS_ERRORS)
    target_compile_options(jiglib PRIVATE -Werror)
  endif()
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${JIGLIB_SOURCES} ${JIGLIB_HEADERS})
