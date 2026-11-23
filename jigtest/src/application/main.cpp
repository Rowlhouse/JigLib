//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
#include "jigtestapp.hpp"

#include "jiglib.hpp"

#include <SDL3/SDL_main.h>

#include <filesystem>
#include <string>

using namespace std;
using namespace JigLib;

//======================================================================================================================
// main
//======================================================================================================================
int main(int argc, char * argv[])
{
  // The config files and data/ live next to the executable. If they aren't in the current directory
  // (e.g. when launched from a file browser or IDE) then switch to the executable's directory.
  if (!std::filesystem::exists("jigtest.cfg"))
  {
    if (const char * basePath = SDL_GetBasePath())
    {
      std::error_code ec;
      std::filesystem::current_path(basePath, ec);
    }
  }

  // Scope this to control when the app gets deleted
  {
    tJigTestApp jigTestApp(argc, argv);

    jigTestApp.Initialise("JigTest");
    jigTestApp.StartMainLoop();
  }

  TRACE_FILE_IF(ONCE_1)
    TRACE("Program exiting normally\n");

  return 0;
}
