JigLib is a rigid body physics library. See ../JigTest for a demo
application that uses it.

This folder is self-contained. To use JigLib in your own application,
copy this folder into your project and, in your CMakeLists.txt:

  add_subdirectory(JigLib)
  target_link_libraries(your_app PRIVATE JigLib::jiglib)

Then include "jiglib.hpp". It can also be built on its own:

  cmake -S . -B build
  cmake --build build

Options: JIGLIB_USE_DOUBLE (double precision) and
JIGLIB_WARNINGS_AS_ERRORS.

Coding style:

Types defined by JigLib start with a "t" and the first letter of each
word is capitalised - e.g. tCollSkin.

Variables used by JigLib start with lowercase, and the first letter of
each subsequent word is capitalised - e.g. collSkin.

Member variables start with "m" - e.g. mCollSkin. No more than this -
i.e. NOT mpCollSkin

Functions start with a capital letter and the first letter of each
word is capitalised.

Macros are in capitals (unless they're pretending not to be macros)
and underscore is used to separate words.

Everything is in namespace "JigLib". Code inside a top-level namespace
is not indented.

Indentation is two spaces - NOT TABS

Lines are at most 120 characters. Comments use // (not /// or /* */)
and are wrapped at 100 characters. Separator lines (//====) extend to
120 characters.

Every header/file has to have a newline at the end (MSVC doesn't
enforce this, but gcc does!).

JigLib is for physics - it shouldn't contain any render code! If you
want to see something, use another library or write render code at the
application level.

