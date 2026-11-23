//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "sdlapplicationbase.hpp"
#include "graphics.hpp"
#include "imageloader.hpp"
#include "resources.hpp"
#include <stdio.h>
#include <filesystem>
#include <string.h>
#include <vector>

using namespace std;
using namespace JigLib;

static SDL_Window * sMainWindow = 0;

//======================================================================================================================
// GetMainWindow
//======================================================================================================================
SDL_Window * GetMainWindow()
{
  return sMainWindow;
}

void tSDLApplicationBase::LoadConfig(int & argc, char * argv[],
                                     string configFileName)
{
  // Setup config/tracing
  bool configFileOk;
  if (argc > 1)
    configFileName = string(argv[1]);
  configFileName = GetConfigPath(configFileName);
  mConfigFile = new tConfigFile(configFileName, configFileOk);
  mConfigName = std::filesystem::path(configFileName).stem().string();

  if (!configFileOk)
    TRACE("Warning: Unable to open main config file: %s\n", configFileName.c_str());

  // Initialise trace. Set up tracing properly
  bool traceEnabled = true;
  int traceLevel = 3;
  bool traceAllStrings = true;
  vector<string> traceStrings;

  mConfigFile->GetValue("trace_enabled", traceEnabled);
  mConfigFile->GetValue("trace_level", traceLevel);
  mConfigFile->GetValue("trace_all_strings", traceAllStrings);
  mConfigFile->GetValues("trace_strings", traceStrings);

  EnableTrace(traceEnabled);
  SetTraceLevel(traceLevel);
  EnableTraceAllStrings(traceAllStrings);
  AddTraceStrings(traceStrings);

  TRACE_FILE_IF(ONCE_1)
    TRACE("Logging set up\n");
}


//======================================================================================================================
// LoadConfigFile
//======================================================================================================================
bool tSDLApplicationBase::LoadConfigFile(const std::string & name)
{
  std::string path = GetConfigPath(name);
  bool ok = false;
  tConfigFile * configFile = new tConfigFile(path, ok);
  if (!ok)
  {
    TRACE("Warning: Unable to open config file: %s\n", path.c_str());
    delete configFile;
    return false;
  }
  delete mConfigFile;
  mConfigFile = configFile;
  mConfigName = std::filesystem::path(path).stem().string();
  return true;
}

//======================================================================================================================
// tSDLApplicationBase
//======================================================================================================================
tSDLApplicationBase::tSDLApplicationBase(int & argc, char * argv[],
                                         string configFileName,
                                         void (* licenseFn)(void))
  :
  mWindow(0),
  mGLContext(0)
{
  TRACE_METHOD_ONLY(ONCE_1);

  LoadConfig(argc, argv, configFileName);

  if (licenseFn)
    (*licenseFn)();
  else
    DisplayLicense();
}

//======================================================================================================================
// ~tSDLApplicationBase
//======================================================================================================================
tSDLApplicationBase::~tSDLApplicationBase()
{
  TRACE_METHOD_ONLY(ONCE_1);
  if (mGLContext)
    SDL_GL_DestroyContext(mGLContext);
  if (mWindow)
    SDL_DestroyWindow(mWindow);
  sMainWindow = 0;
}

//======================================================================================================================
// Initialise
//======================================================================================================================
bool tSDLApplicationBase::Initialise(const std::string app_name)
{
  TRACE_METHOD_ONLY(ONCE_1);

  // Intialise some vars
  mStartOfFrameTime = tTime(0.0f);
  mOldStartOfFrameTime = tTime(0.0f);
  mFPSInterval = tTime(1.0f);
  mLastStoredTime = tTime(0.0f);
  mFPSCounter = 0;
  mFPS = 1.0f;
  mNewFPS = true;

  TRACE_FILE_IF(ONCE_2)
    TRACE("Initializing SDL.\n");
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    TRACE("Could not initialize SDL: %s.\n", SDL_GetError());
    return false;
  }
  // Clean up on exit
  atexit(SDL_Quit);

  TRACE_FILE_IF(ONCE_2)
    TRACE("SDL initialized.\n");

  // A size of 0 means fit the screen. Screens vary so much that a fixed default is always either
  // too big or too small.
  mWindowWidth = 0;
  mWindowHeight = 0;
  mConfigFile->GetValue("window_width", mWindowWidth);
  mConfigFile->GetValue("window_height", mWindowHeight);
  if (mWindowWidth <= 0 || mWindowHeight <= 0)
  {
    SDL_Rect usable;
    if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable))
    {
      mWindowWidth = (int) (0.8f * usable.w);
      mWindowHeight = (int) (0.8f * usable.h);
    }
    else
    {
      mWindowWidth = 1280;
      mWindowHeight = 720;
    }
  }

  // The rendering uses the fixed-function pipeline, so we need a compatibility (or legacy) context,
  // which is what we get by default.
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  bool fullscreen = false;
  mConfigFile->GetValue("fullscreen", fullscreen);

  SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  if (fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN;
  mWindow = SDL_CreateWindow(app_name.c_str(), mWindowWidth, mWindowHeight, flags);
  if (!mWindow)
  {
    TRACE("Window creation failed: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  mGLContext = SDL_GL_CreateContext(mWindow);
  if (!mGLContext)
  {
    TRACE("OpenGL context creation failed: %s\n", SDL_GetError());
    return false;
  }
  SDL_GL_SetSwapInterval(1);
  sMainWindow = mWindow;

  TRACE_FILE_IF(ONCE_2)
    TRACE("OpenGL %s: %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

  return true;
}

//======================================================================================================================
// GetTimeNow
//======================================================================================================================
tTime tSDLApplicationBase::GetTimeNow() const
{
  Uint64 millisec = SDL_GetTicks();
  return millisec * 0.001f;
}

//======================================================================================================================
// InternalHandleStartOfLoop
//======================================================================================================================
void tSDLApplicationBase::InternalHandleStartOfLoop()
{
  mOldStartOfFrameTime = mStartOfFrameTime;
  mStartOfFrameTime = GetTimeNow();
  if ( (mStartOfFrameTime > 0.0f) &&
       (mStartOfFrameTime < mFPSInterval) )
  {
    mFPS = mFPSCounter / mStartOfFrameTime;
  }

  if (mStartOfFrameTime - mLastStoredTime >= mFPSInterval)
  {
    mFPS = (tScalar) mFPSCounter;
    mLastStoredTime = mStartOfFrameTime;
    mFPSCounter = 0;
    mNewFPS = true;
  }
  else
  {
    ++mFPSCounter;
    mNewFPS = false;
  }
}


//======================================================================================================================
// StartMainLoop
//======================================================================================================================
void tSDLApplicationBase::StartMainLoop()
{
  mExitMainLoop = false;

  while( false == mExitMainLoop )
  {
    InternalHandleStartOfLoop();
    // Do all the basic stuff like keyboard, mouse etc, calling the
    // app.
    ProcessEvents();
    // Get the app to do it's own stuff - e.g. physics, rendering
    ProcessMainEvent();
  }

}

//======================================================================================================================
// InternalHandleResize
//======================================================================================================================
void tSDLApplicationBase::InternalHandleResize(int newWidth, int newHeight)
{
  // The viewports pick up the new size when they're next applied
  mWindowWidth = newWidth;
  mWindowHeight = newHeight;
  HandleReShape(newWidth, newHeight);
}


//======================================================================================================================
// ProcessEvents
//======================================================================================================================
void tSDLApplicationBase::ProcessEvents()
{
  SDL_Event event;

  while( SDL_PollEvent( &event ) )
  {
    switch( event.type ) {
    case SDL_EVENT_KEY_DOWN:
      // Ignore auto-repeat, so held keys don't keep toggling things
      if (!event.key.repeat)
        HandleKeyDown( event.key );
      break;
    case SDL_EVENT_KEY_UP:
      HandleKeyUp( event.key );
      break;
    case SDL_EVENT_MOUSE_MOTION:
      HandleMouseMotion( event.motion );
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      HandleMouseButtonDown( event.button );
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      HandleMouseButtonUp( event.button );
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      HandleMouseWheel( event.wheel );
      break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      InternalHandleResize( event.window.data1, event.window.data2 );
      break;
    case SDL_EVENT_QUIT:
      // Handle quit requests (like Ctrl-c).
      exit(0);
      break;
    }
  }
}

//======================================================================================================================
// HandleReShape
//======================================================================================================================
void tSDLApplicationBase::HandleReShape(int newWidth, int newHeight)
{
}


//======================================================================================================================
// DisplayLicense
//======================================================================================================================
void tSDLApplicationBase::DisplayLicense()
{
  TRACE("Application Copyright 2004 Danny Chapman: https://www.rowlhouse.co.uk\n");
  TRACE("Application comes with ABSOLUTELY NO WARRANTY;\n");
  TRACE("This is free software, and you are welcome to redistribute it\n");
  TRACE("under certain conditions; see the zlib license in license.txt\n");
}



//======================================================================================================================
// WriteFrameBuffer
//======================================================================================================================
static void WriteFrameBuffer(const char *filename)
{
  int width, height;
  GetWindowSize(width, height);
  if (width <= 0 || height <= 0)
    return;

  std::vector<unsigned char> framebuffer(width * height * 3);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &framebuffer[0]);

  // OpenGL's first row is the bottom of the image, but PNG's is the top
  std::vector<unsigned char> flipped(framebuffer.size());
  for (int y = 0 ; y < height ; ++y)
    memcpy(&flipped[y * width * 3], &framebuffer[(height - y - 1) * width * 3], width * 3);

  if (!SavePNGImage(filename, width, height, &flipped[0]))
    TRACE("Unable to write screenshot %s\n", filename);
}

//======================================================================================================================
// DoScreenshot
//======================================================================================================================
void tSDLApplicationBase::DoScreenshot()
{
  static int count = 0;
  char screen_file[64];
  snprintf(screen_file, sizeof(screen_file), "screenshot-%05d.png", count++);
  WriteFrameBuffer(screen_file);
}



