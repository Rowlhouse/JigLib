//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
#include "jigtestapp.hpp"

#include "jiglib.hpp"

#include <SDL3/SDL_main.h>

#include <string>

using namespace std;
using namespace JigLib;

//======================================================================================================================
// main
//======================================================================================================================
int main(int argc, char * argv[])
{
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
