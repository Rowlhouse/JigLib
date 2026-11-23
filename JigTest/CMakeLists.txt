# JigTest - the graphical demo/test application for JigLib.
#
# Dependencies are JigLib, SDL3, OpenGL, stb_image and pugixml. SDL3 and pugixml are used from the
# system if CMake can find them, otherwise they (and stb) are downloaded and built as part of this
# project.
#
# This is normally built from the top-level CMakeLists.txt, but can be built on its own too.

cmake_minimum_required(VERSION 3.21)

project(JigTest LANGUAGES C CXX)

if(NOT TARGET JigLib::jiglib)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JigLib ${CMAKE_CURRENT_BINARY_DIR}/JigLib)
endif()

include(FetchContent)

find_package(OpenGL REQUIRED)

# SDL3
find_package(SDL3 3.2 CONFIG QUIET)
if(SDL3_FOUND)
  message(STATUS "JigTest: using installed SDL3 ${SDL3_VERSION}")
else()
  message(STATUS "JigTest: SDL3 not found - it will be downloaded and built")
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz)
  FetchContent_MakeAvailable(SDL3)
endif()

# pugixml
find_package(pugixml CONFIG QUIET)
if(NOT pugixml_FOUND)
  set(PUGIXML_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(pugixml
    URL https://github.com/zeux/pugixml/releases/download/v1.16/pugixml-1.16.tar.gz)
  FetchContent_MakeAvailable(pugixml)
endif()

# stb (header only - stb_image and stb_image_write)
FetchContent_Declare(stb
  URL https://github.com/nothings/stb/archive/0e9206afd9752c8c0826b671b16b702ec7d175cd.tar.gz)
FetchContent_MakeAvailable(stb)
add_library(jigtest_stb INTERFACE)
target_include_directories(jigtest_stb SYSTEM INTERFACE ${stb_SOURCE_DIR})

add_executable(jigtest
  src/application/appconfig.cpp
  src/application/boxchain.cpp
  src/application/boxobject.cpp
  src/application/boxsphereobject.cpp
  src/application/capsuleobject.cpp
  src/application/carai.cpp
  src/application/carobject.cpp
  src/application/characterobject.cpp
  src/application/contactrender.cpp
  src/application/debugconfig.cpp
  src/application/heightmapobject.cpp
  src/application/jigtestapp.cpp
  src/application/main.cpp
  src/application/meshobject.cpp
  src/application/newton.cpp
  src/application/object.cpp
  src/application/planeobject.cpp
  src/application/ragdoll.cpp
  src/application/spherechain.cpp
  src/application/sphereobject.cpp
  src/framework/camera.cpp
  src/framework/glutils.cpp
  src/framework/imageloader.cpp
  src/framework/rendermanager.cpp
  src/framework/renderobject.cpp
  src/framework/resources.cpp
  src/framework/sdlapplicationbase.cpp
  src/framework/skybox.cpp
  src/framework/texture.cpp
  src/framework/viewport.cpp
)

target_include_directories(jigtest PRIVATE src/application src/framework)
target_link_libraries(jigtest PRIVATE JigLib::jiglib SDL3::SDL3 OpenGL::GL pugixml::pugixml jigtest_stb)

if(APPLE)
  # OpenGL is deprecated on macOS but still works
  target_compile_definitions(jigtest PRIVATE GL_SILENCE_DEPRECATION)
endif()

if(MSVC)
  target_compile_definitions(jigtest PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(jigtest PRIVATE /W3 /wd4267 /permissive-)
  if(JIGLIB_WARNINGS_AS_ERRORS)
    target_compile_options(jigtest PRIVATE /WX)
  endif()
else()
  target_compile_options(jigtest PRIVATE -Wall)
  if(JIGLIB_WARNINGS_AS_ERRORS)
    target_compile_options(jigtest PRIVATE -Werror)
  endif()
endif()

# The demo looks for configs/ and data/ next to the executable (for an installed copy), and then in
# this source folder (for a development build), so nothing needs copying into the build folder.
target_compile_definitions(jigtest PRIVATE JIGTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT jigtest)

# cmake --install <build dir> --prefix <folder> gives a self-contained, runnable copy of the demo
install(TARGETS jigtest RUNTIME DESTINATION .)
install(DIRECTORY configs data DESTINATION .)
install(FILES readme.txt ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt DESTINATION .)

