# JigLib

JigLib is a rigid body physics library written in C++. I wrote it between 2004 and 2007, along with
JigTest, an interactive demo that shows it in action. It was originally available as a zip from my
website. This repository keeps it in one place, updated so that it builds with current compilers on
Windows, Linux and macOS.

It isn't under active development. The version number is 0.84, and a comment in the code says
"Let's face facts... We're not going to reach 1!!"

## Features

- Rigid bodies with collision skins built from boxes, spheres, capsules and planes, plus static
  heightmaps and triangle meshes (using an octree).
- Collision detection between all of these, with either a brute-force or a grid-based broad phase.
- Ray/segment intersection tests against the world.
- Constraints for point-to-point and maximum distance joints, fixing a point to the world, driving
  a velocity, and hinges.
- Several iterative solvers for collisions and contacts, an optional shock propagation step for
  stacking, and freezing of objects that have come to rest.
- Per-material elasticity and friction.
- Cars with ray-cast wheels and spring/damper suspension.
- Single or double precision.

The library is plain C++ with no dependencies, and it doesn't do any rendering. JigTest uses SDL3
and OpenGL to show the simulation, with boxes, spheres, capsules, ragdolls, cars, a Newton's cradle
and a castle mesh to play with.

## Repository layout

| Folder | Contents |
| --- | --- |
| `JigLib/` | The physics library. It's self-contained and has its own `CMakeLists.txt`. |
| `JigTest/` | The demo application, with its `configs/` and `data/`. |
| `JigLib/tests/` | A small smoke test, which is also a minimal example of using JigLib. |

The top-level `CMakeLists.txt` builds both. If you delete `JigTest/`, it just builds the library.

## Building

You need CMake 3.21 or later and a C++17 compiler (Visual Studio 2022 or 2026, GCC or Clang).
JigTest also needs OpenGL. It uses SDL3 and pugixml if CMake can find them, and otherwise
downloads and builds them, so a first build takes a little longer.

```sh
cmake --preset default
cmake --build --preset release
ctest --preset release
```

On Windows, run these from a Visual Studio developer prompt, or open the folder in Visual Studio,
which picks up the presets. If you'd rather have a solution file, use `cmake --preset vs2026` (or
`vs2022`) and open the `.sln` in `build/vs2026`.

On Linux, building SDL3 needs the usual X11/Wayland and OpenGL development packages. The CI
workflow in `.github/workflows/ci.yml` lists them for Ubuntu.

## Running JigTest

The executable ends up in `build/default/JigTest/Release`. It finds its config files and data in
the `JigTest` folder, so you can run it from anywhere.

```sh
jigtest            # configs/default.cfg
jigtest castle     # or demo1, demo2, newton, stress
```

Press `x`, `z` or `n` to drop in a box, sphere or capsule (hold Ctrl to fire it from the camera),
`\` for a ragdoll and `c` for a car you can drive with the arrow keys. Move the camera with
`w` `a` `s` `d` `q` `e`, pick objects up with the right mouse button, and press `t` to reset.
`JigTest/readme.txt` has the full list of keys. The config files are plain text and control most
of the scene and physics settings.

For a self-contained copy of the demo, with its configs and data next to the executable, use
`cmake --install build/default --config Release --prefix <folder>`.

## Using JigLib in your own code

Copy the `JigLib` folder into your project, then in your `CMakeLists.txt`

```cmake
add_subdirectory(JigLib)
target_link_libraries(your_app PRIVATE JigLib::jiglib)
```

and include `jiglib.hpp`. Everything is in the `JigLib` namespace. A simulation has a
`tPhysicsSystem`, a collision system (such as `tCollisionSystemGrid`), and `tBody` objects that
each have a `tCollisionSkin` made from one or more primitives. You step it by calling
`tPhysicsSystem::Integrate`. See `JigLib/tests/smoketest.cpp` for a complete example in about a
hundred lines, and the JigTest source for much more.

## History

JigLib has been ported to other languages. JigLibX is a C# port for Microsoft's XNA, and
[JigLibFlash](https://code.google.com/archive/p/jiglibflash) is an ActionScript 3 port for Flash.
JigLibFlash was in turn ported to JavaScript ([JigLibJS](https://github.com/supereggbert/JigLibJS))
and Haxe ([JigLibHX](https://github.com/deprecated-armory3d/jiglibhx)).

The tag `v0.84-original` holds the source as it was in its original Subversion repository, without
the build outputs and bundled third-party libraries. These are the main changes since then.

- The old Visual Studio projects and makefiles are replaced by CMake.
- The demo has moved from SDL 1.2, GLU, libpng and libxml2 to SDL3, stb_image and pugixml.
- The comments and layout are reformatted, and spelling is fixed.
- A few bugs found by modern compilers and the new smoke test are fixed.
- Continuous integration builds and tests everything on Windows, Linux and macOS.

Apart from those bug fixes, the physics code is as it was.

## Licence

JigLib and JigTest are released under the zlib licence. See [LICENSE.txt](LICENSE.txt).
