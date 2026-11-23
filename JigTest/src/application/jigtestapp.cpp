//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================

#include "jigtestapp.hpp"
#include "appconfig.hpp"
#include "debugconfig.hpp"
#include "sphereobject.hpp"
#include "capsuleobject.hpp"
#include "boxobject.hpp"
#include "boxsphereobject.hpp"
#include "planeobject.hpp"
#include "heightmapobject.hpp"
#include "meshobject.hpp"
#include "carobject.hpp"
#include "characterobject.hpp"
#include "ragdoll.hpp"
#include "newton.hpp"
#include "boxchain.hpp"
#include "spherechain.hpp"
#include "viewport.hpp"
#include "camera.hpp"
#include "contactrender.hpp"
#include "skybox.hpp"
#include "textoverlay.hpp"

#include <algorithm>

using namespace JigLib;
using namespace std;

//======================================================================================================================
// DisplayLicense
//======================================================================================================================
void DisplayLicense()
{
  TRACE("JigTest Copyright 2007 Danny Chapman: https://www.rowlhouse.co.uk\n");
  TRACE("This comes with ABSOLUTELY NO WARRANTY;\n");
  TRACE("This is free software, and you are welcome to redistribute it\n");
  TRACE("under certain conditions; see the zlib license in license.txt\n");
  TRACE("Version compiled " __TIME__ " " __DATE__ "\n");
}

//======================================================================================================================
// tJigTestApp
//======================================================================================================================
tJigTestApp::tJigTestApp(int & argc, char * argv[])
  :
  tSDLApplicationBase(argc, argv, "", ::DisplayLicense),
  mCarAI(100.0f)
{
  TRACE_METHOD_ONLY(ONCE_1);
  mSelectedIndex = -1;
  mCameraMoveDir = MOVE_NONE;
  mCameraMoveVel.SetTo(0.0f);
  mCameraYaw = 0.0f;
  mCameraPitch = 0.0f;
  mCameraLookDir = 0;
  mMouseMode = MOUSE_NONE;
  mMousePos = tVector2(0.0f, 0.0f);
  mOrbitPoint = tVector3::Zero();
  mShowHelp = true;
  mPhysicsDuration = 0.0f;
  mPaused = false;
  mPauseAfterNextPhysicsStep = false;
  mCollisionSystem = 0;
  mSkybox = 0;
  mPicking = false;
  mDoMovie = false;
  mQuitIterations = 0;
  mPhysicsCounter = 0;
}

//======================================================================================================================
// ~tJigTestApp
//======================================================================================================================
tJigTestApp::~tJigTestApp()
{
  TRACE_METHOD_ONLY(ONCE_1);

  unsigned i;
  for (i = 0 ; i < mObjects.size() ; ++i)
    delete mObjects[i];
  for (i = 0 ; i < mCameras.size() ; ++i)
    delete mCameras[i];
  for (i = 0 ; i < mViewports.size() ; ++i)
    delete mViewports[i];

  delete mCollisionSystem;
}

//======================================================================================================================
// HandleFPSMouseMotion
//======================================================================================================================
void tJigTestApp::HandleFPSMouseMotion(const SDL_MouseMotionEvent & event)
{
  TRACE_METHOD_ONLY(FRAME_1);
  static bool handlingWarp = false;
  if (handlingWarp)
  {
    handlingWarp = false;
    return;
  }

  if (mCharacterObjects.empty())
    return;

  tCharacterObject* character = mCharacterObjects[0];
  tBody* body = character->GetBody();
  if (!body) return;

  float x, y;
  SDL_GetMouseState(&x, &y);
  int w, h;
  if (SDL_GetWindowSize(GetMainWindow(), &w, &h))
  {
    float midX = w / 2.0f;
    float midY = h / 2.0f;

    handlingWarp = true;
    SDL_WarpMouseInWindow(GetMainWindow(), midX, midY);

    float bearingScale = 30.0f;
    float deltaBearing = (bearingScale * (x - midX)) / midX;

    tVector3 origLook = body->GetOrientation().GetLook();
    origLook.z = 0.0f;

    tVector3 newLook = Matrix33Gamma(-deltaBearing) * origLook;
    newLook.NormaliseSafe();

    body->SetOrientation(tMatrix33(newLook, Cross(tVector3::Up(), newLook), tVector3::Up()));
  }
}


//======================================================================================================================
// HandleFPSKeyDown
//======================================================================================================================
void tJigTestApp::HandleFPSKeyDown( const SDL_KeyboardEvent & event )
{
  TRACE_METHOD_ONLY(FRAME_1);
  if (mCharacterObjects.empty())
    return;

  tCharacterObject* character = mCharacterObjects[0];
  switch( event.key )
  {
  case SDLK_F1:
    DoScreenshot();
    break;
  case SDLK_F2:
    mDoMovie = !mDoMovie;
    break;
  case SDLK_H:
    mShowHelp = !mShowHelp;
    break;
  case SDLK_ESCAPE:
    ExitMainLoop();
    break;
  case SDLK_Y:
    RemoveAllObjects(true);
    break;
  case SDLK_T:
    RestartExperiment();
    break;
  case SDLK_W: character->ControlFwd(true); break;
  case SDLK_A: character->ControlLeft(true); break;
  case SDLK_S: character->ControlBack(true); break;
  case SDLK_D: character->ControlRight(true); break;
  case SDLK_SPACE: character->ControlJump(); break;
  default: break;
  }
}

//======================================================================================================================
// HandleFPSKeyUp
//======================================================================================================================
void tJigTestApp::HandleFPSKeyUp( const SDL_KeyboardEvent & event )
{
  TRACE_METHOD_ONLY(FRAME_1);
  if (mCharacterObjects.empty())
    return;

  tCharacterObject* character = mCharacterObjects[0];

  switch( event.key )
  {
  case SDLK_W: character->ControlFwd(false); break;
  case SDLK_A: character->ControlLeft(false); break;
  case SDLK_S: character->ControlBack(false); break;
  case SDLK_D: character->ControlRight(false); break;
  default: break;
  }
}

//======================================================================================================================
// HandleKeyDown
//======================================================================================================================
void tJigTestApp::HandleKeyDown( const SDL_KeyboardEvent & event )
{
  TRACE_METHOD_ONLY(FRAME_1);

  if (!mCharacterObjects.empty())
  {
    HandleFPSKeyDown(event);
    return;
  }

  SDL_Keymod keyMod = SDL_GetModState();
  tCarObject * car = GetSelectedCar();

  switch( event.key )
  {
  case SDLK_F1:
    DoScreenshot();
    break;
  case SDLK_F2:
    mDoMovie = !mDoMovie;
    break;
  case SDLK_H:
    mShowHelp = !mShowHelp;
    break;
  case SDLK_W:
    mCameraMoveDir |= MOVE_FWD;
    break;
  case SDLK_S:
    mCameraMoveDir |= MOVE_BACK;
    break;
  case SDLK_A:
    mCameraMoveDir |= MOVE_LEFT;
    break;
  case SDLK_D:
    mCameraMoveDir |= MOVE_RIGHT;
    break;
  case SDLK_E:
    mCameraMoveDir |= MOVE_UP;
    break;
  case SDLK_Q:
    mCameraMoveDir |= MOVE_DOWN;
    break;
  case SDLK_R:
    tAppConfig::mRenderEnable = !tAppConfig::mRenderEnable;
    break;
  case SDLK_K:
    tDebugConfig::mRenderCollisionContacts =
      (tDebugConfig::mRenderCollisionContacts + 1) %3;
    break;
  case SDLK_B:
    tDebugConfig::mRenderCollisionBoundingBoxes =
      !tDebugConfig::mRenderCollisionBoundingBoxes;
    break;
  case SDLK_V:
    tDebugConfig::mIndicateFrozenObjects =
      !tDebugConfig::mIndicateFrozenObjects;
    break;
  case SDLK_F:
    tAppConfig::mEnableFreezing = !tAppConfig::mEnableFreezing;
    mPhysics.EnableFreezing(tAppConfig::mEnableFreezing);
    break;
  case SDLK_G:
    tAppConfig::mDoShockStep = !tAppConfig::mDoShockStep;
    mPhysics.SetDoShockStep(tAppConfig::mDoShockStep);
    break;
  case SDLK_P:
    mPaused = !mPaused;
    break;
  case SDLK_O:
    mPaused = false;
    mPauseAfterNextPhysicsStep = true;
    break;
  case SDLK_EQUALS:
    SelectNextObject(1);
    break;
  case SDLK_MINUS:
    SelectNextObject(-1);
    break;
  case SDLK_SPACE:
    FireObject(mSelectedIndex);
    break;
  case SDLK_X:
    CreateBox();
    if (keyMod & SDL_KMOD_CTRL)
      FireLastCreatedObject();
    break;
  case SDLK_SLASH:
    CreateCompound();
    if (keyMod & SDL_KMOD_CTRL)
      FireLastCreatedObject();
    break;
  case SDLK_Z:
    CreateSphere();
    if (keyMod & SDL_KMOD_CTRL)
      FireLastCreatedObject();
    break;
  case SDLK_N:
    CreateCapsule();
    if (keyMod & SDL_KMOD_CTRL)
      FireLastCreatedObject();
    break;
  case SDLK_C:
    CreateCar();
    if (keyMod & SDL_KMOD_CTRL)
      FireLastCreatedObject();
    break;
  case SDLK_BACKSLASH:
  case SDLK_LESS:
    CreateRagdoll();
    if (keyMod & SDL_KMOD_CTRL)
      FireLastCreatedObject();
    break;
  case SDLK_M:
    CreateCharacter();
    break;
  case SDLK_Y:
    RemoveAllObjects(true);
    break;
  case SDLK_T:
    RestartExperiment();
    break;
  // The arrow keys drive the selected car, or otherwise turn the camera
  case SDLK_LEFT:
    if (car)
      ControlCar(car, CONTROL_LEFT, true);
    else
      mCameraLookDir |= LOOK_LEFT;
    break;
  case SDLK_RIGHT:
    if (car)
      ControlCar(car, CONTROL_RIGHT, true);
    else
      mCameraLookDir |= LOOK_RIGHT;
    break;
  case SDLK_UP:
    if (car)
      ControlCar(car, CONTROL_UP, true);
    else
      mCameraLookDir |= LOOK_UP;
    break;
  case SDLK_DOWN:
    if (car)
      ControlCar(car, CONTROL_DOWN, true);
    else
      mCameraLookDir |= LOOK_DOWN;
    break;
  case SDLK_ESCAPE:
    ExitMainLoop();
    break;
  case SDLK_PAGEDOWN:
    tAppConfig::mTextureLevel -= 1;
    Limit(tAppConfig::mTextureLevel, -1, 1);
    mRenderManager.SetWireFrameMode(tAppConfig::mTextureLevel == -1);
    break;
  case SDLK_PAGEUP:
    tAppConfig::mTextureLevel += 1;
    Limit(tAppConfig::mTextureLevel, -1, 1);
    mRenderManager.SetWireFrameMode(tAppConfig::mTextureLevel == -1);
    break;
  default:
    break;
  }
  SetSleepingParams();
}

//======================================================================================================================
// HandleKeyUp
//======================================================================================================================
void tJigTestApp::HandleKeyUp( const SDL_KeyboardEvent & event )
{
  TRACE_METHOD_ONLY(FRAME_1);

  if (!mCharacterObjects.empty())
  {
    HandleFPSKeyUp(event);
    return;
  }

  tCarObject * car = GetSelectedCar();

  switch( event.key )
  {
  case SDLK_W:
    mCameraMoveDir &= ~MOVE_FWD;
    break;
  case SDLK_S:
    mCameraMoveDir &= ~MOVE_BACK;
    break;
  case SDLK_A:
    mCameraMoveDir &= ~MOVE_LEFT;
    break;
  case SDLK_D:
    mCameraMoveDir &= ~MOVE_RIGHT;
    break;
  case SDLK_E:
    mCameraMoveDir &= ~MOVE_UP;
    break;
  case SDLK_Q:
    mCameraMoveDir &= ~MOVE_DOWN;
    break;
  case SDLK_LEFT:
    mCameraLookDir &= ~LOOK_LEFT;
    if (car)
      ControlCar(car, CONTROL_LEFT, false);
    break;
  case SDLK_RIGHT:
    mCameraLookDir &= ~LOOK_RIGHT;
    if (car)
      ControlCar(car, CONTROL_RIGHT, false);
    break;
  case SDLK_UP:
    mCameraLookDir &= ~LOOK_UP;
    if (car)
      ControlCar(car, CONTROL_UP, false);
    break;
  case SDLK_DOWN:
    mCameraLookDir &= ~LOOK_DOWN;
    if (car)
      ControlCar(car, CONTROL_DOWN, false);
    break;
  default:
    break;
  }
}

//======================================================================================================================
// HandleMouseButtonDown
//======================================================================================================================
void tJigTestApp::HandleMouseButtonDown( const SDL_MouseButtonEvent & event )
{
  if (!GetPhysicsSystem())
    return;

  mMousePos = tVector2(event.x, event.y);

  if (event.button == SDL_BUTTON_MIDDLE)
  {
    // Throw the object that's being dragged
    if (mPicking)
    {
      if (tBody * body = mObjectPositionController.GetBody())
      {
        StopDrag();
        const tVector3 & camPos = mCameras[0]->GetPosition();
        tVector3 dir = (body->GetPosition() - camPos).GetNormalisedSafe();
        body->SetVelocity(tAppConfig::mProjectileSpeed * dir);
      }
    }
    return;
  }

  if (event.button == SDL_BUTTON_RIGHT)
  {
    StartMouseLook(MOUSE_LOOK);
    return;
  }

  if (event.button != SDL_BUTTON_LEFT)
    return;

  if (SDL_GetModState() & SDL_KMOD_ALT)
  {
    StartMouseLook(MOUSE_ORBIT);
    return;
  }

  // A left click selects whatever is under the mouse, and starts dragging it
  tVector3 pos;
  tBody * body = 0;
  if (!PickPoint(mMousePos, pos, body))
  {
    SelectObject(-1);
    return;
  }
  mOrbitPoint = pos;

  int index = -1;
  if (body)
  {
    for (unsigned i = 0 ; i < mObjects.size() ; ++i)
    {
      if (mObjects[i]->GetBody() == body)
      {
        index = (int) i;
        break;
      }
    }
  }
  SelectObject(index);

  if (!body)
    return;

  const tVector3 & camPos = mCameras[0]->GetPosition();
  mPickDist = (pos - camPos).GetLength();

  // Need to get the position in body coords
  tVector3 delta = pos - body->GetPosition();
  delta = body->GetOrientation().GetTranspose() * delta;
  body->SetActive();
  mObjectPositionController.Initialise(body, delta, pos);
  mObjectPositionDamperController.Initialise(body, tConstraintVelocity::BODY, 0, &tVector3::Zero());
  mObjectPositionController.EnableConstraint();
  mObjectPositionDamperController.EnableConstraint();
  mPicking = true;
}

//======================================================================================================================
// HandleMouseButtonUp
//======================================================================================================================
void tJigTestApp::HandleMouseButtonUp( const SDL_MouseButtonEvent & event )
{
  if (event.button == SDL_BUTTON_LEFT && mPicking)
    StopDrag();

  if ( (event.button == SDL_BUTTON_RIGHT && mMouseMode == MOUSE_LOOK) ||
       (event.button == SDL_BUTTON_LEFT && mMouseMode == MOUSE_ORBIT) )
    StopMouseLook();
}

//======================================================================================================================
// HandleMouseWheel
//======================================================================================================================
void tJigTestApp::HandleMouseWheel( const SDL_MouseWheelEvent & event )
{
  if (event.y == 0.0f || mCameras.empty())
    return;

  if (mPicking)
  {
    // The wheel moves the dragged object towards/away from the camera
    if (event.y > 0.0f)
      mPickDist *= 1.1f;
    else
      mPickDist *= 0.9f;
    UpdateDrag();
    return;
  }

  // Otherwise it moves the camera forwards/backwards
  tScalar step = (SDL_GetModState() & SDL_KMOD_SHIFT) ? 4.0f : 1.0f;
  tCamera * camera = mCameras[0];
  camera->SetPosition(camera->GetPosition() + (event.y * step) * GetCameraDirection());
}

//======================================================================================================================
// HandleMouseMotion
//======================================================================================================================
void tJigTestApp::HandleMouseMotion( const SDL_MouseMotionEvent & event )
{
  TRACE_METHOD_ONLY(FRAME_1);

  if (!mCharacterObjects.empty())
  {
    HandleFPSMouseMotion(event);
    return;
  }

  if (mMouseMode != MOUSE_NONE)
  {
    // Degrees per pixel
    static const tScalar sensitivity = 0.2f;
    RotateCamera(-sensitivity * event.xrel, -sensitivity * event.yrel, mMouseMode == MOUSE_ORBIT);
    return;
  }

  mMousePos = tVector2(event.x, event.y);
  if (mPicking)
    UpdateDrag();
}


//======================================================================================================================
// ControlCar
//======================================================================================================================
void tJigTestApp::ControlCar(tCarObject * car, tObjectControl control, bool down)
{
  if (!car)
    return;

  car->GetBody()->SetActive();

  switch(control)
  {
  case CONTROL_UP:
    car->GetCar().SetAccelerate(down ? 1.0f : 0.0f);
    break;
  case CONTROL_DOWN:
    car->GetCar().SetAccelerate(down ? -1.0f : 0.0f);
    break;
  case CONTROL_LEFT:
    car->GetCar().SetSteer(down ? 1.0f : 0.0f);
    break;
  case CONTROL_RIGHT:
    car->GetCar().SetSteer(down ? -1.0f : 0.0f);
    break;
  default:
    break;
  }
}

//======================================================================================================================
// FireLastCreatedObject
//======================================================================================================================
void tJigTestApp::FireLastCreatedObject()
{
  if (!mObjects.empty())
    FireObject((int) mObjects.size() - 1);
}

//======================================================================================================================
// FireObject
//======================================================================================================================
void tJigTestApp::FireObject(int index)
{
  if (index < 0 || index >= (int) mObjects.size() || mCameras.empty())
    return;
  tObject * obj = mObjects[index];
  if (!obj->GetBody())
    return;
  tVector3 camPos = mCameras[0]->GetPosition();
  tVector3 dir = (mCameras[0]->GetTarget() - camPos).GetNormalisedSafe();
  obj->SetPhysicsPosition(camPos + 0.001f * dir, obj->GetBody()->GetOrientation());
  obj->SetPhysicsVelocity(tAppConfig::mProjectileSpeed * dir);
}

//======================================================================================================================
// ProcessMainEvent
//======================================================================================================================
void tJigTestApp::ProcessMainEvent()
{
  TRACE_METHOD_ONLY(FRAME_1);
  if (!mPaused && GetJustCalculatedNewFPS())
  {
//    TRACE_FILE_IF(ONCE_1)
    {
      TRACE("FPS = %5.2f, ", GetFPS());
      TRACE("Physics duration = %5.2fms which is %5.2f%% of the timestep\n",
            1000.0f * mPhysicsDuration,
            100.0f * mPhysicsDuration * tAppConfig::mPhysicsFrequency);
    }
  }

  TRACE_FILE_IF(ONCE_1)
    {
      static unsigned oldNumObjects = 0;
      if (mObjects.size() != oldNumObjects)
        TRACE("New number of objects = %d\n", mObjects.size());
      oldNumObjects = mObjects.size();
    }


  CheckGLErrors("Start of ProcessMainEvent");

  int iLoop = 0;

  if (mDoMovie)
  {
    iLoop = 1;
    if (!mPaused)
    {
      for (iLoop = 0 ; iLoop < tAppConfig::mMovieInterval ; ++iLoop)
      {
        mPhysics.Integrate(1.0f / tAppConfig::mPhysicsFrequency);
      }
      mPhysics.ResetTime(GetStartOfFrameTime());
      DoScreenshot();
    }
  }
  else
  {
    if (!mPaused)
    {
      mPhysics.SetNullUpdate(tAppConfig::mNullPhysicsUpdate);
      // The idea is that we run physics until it progresses just beyond where we want to be.
      int maxLoops = tAppConfig::mMaxPhysicsStepsPerFrame;
      while ( (iLoop < maxLoops) &&
              (GetStartOfFrameTime() > mPhysics.GetTargetTime()) )
      {
        ++iLoop;
        bool timeAvailable = false;
        tHighResTimeVal before = GetHighResTime(timeAvailable);

        mPhysics.Integrate(1.0f / tAppConfig::mPhysicsFrequency);
        ++mPhysicsCounter;

        tHighResTimeVal after = GetHighResTime(timeAvailable);
        if (timeAvailable)
        {
          static const tScalar frac = 0.05f;
          mPhysicsDuration =
            (1.0f - frac) * mPhysicsDuration +
            frac * GetTimeDelta(before, after);
        }

        if (mQuitIterations != 0 && mPhysicsCounter >= mQuitIterations)
        {
          TRACE("Reached %d physics updates - quitting\n", mPhysicsCounter);
          exit(0);
        }

        // Single stepping?
        if (mPauseAfterNextPhysicsStep)
        {
          mPauseAfterNextPhysicsStep = false;
          mPaused = true;
          break;
        }
      }
      if (GetStartOfFrameTime() > mPhysics.GetTargetTime())
      {
        mPhysics.ResetTime(GetStartOfFrameTime());
        static tTime lastTime = tTime(0.0f);
        if (GetStartOfFrameTime() > lastTime + 1.0f)
        {
          TRACE("Warning - physics can't keep up\n");
          lastTime = GetStartOfFrameTime();
        }
      }
    }
    else
    {
      mPhysics.SetNullUpdate(true);
      mPhysics.Integrate(1.0f / tAppConfig::mPhysicsFrequency);
      mPhysics.ResetTime(GetStartOfFrameTime());
      iLoop = 1;
    }
  }

  // Update car ai
  mCarAI.Update(iLoop / tAppConfig::mPhysicsFrequency);

  // Move the free camera (unless there's a first-person character, which has its own camera)
  if (mCharacterObjects.empty())
  {
    tScalar dt = GetStartOfFrameTime() - GetOldStartOfFrameTime();
    if (mDoMovie)
      dt = 1.0f / tAppConfig::mPhysicsFrequency;
    UpdateFreeCamera(dt);
  }

  // Do rendering
  if (tAppConfig::mRenderEnable)
  {
    // The fraction used to interpolate between old and current positions in the physics when
    // rendering.
    tScalar renderFraction = 1.0f;

    if (tAppConfig::mRenderInterpolate)
    {
      tScalar physicsDt = mPhysics.GetTargetTime() - mPhysics.GetOldTime();
      if (physicsDt > SCALAR_TINY)
      {
        renderFraction =
          (GetStartOfFrameTime() - mPhysics.GetOldTime()) / physicsDt;
        Limit(renderFraction, SCALAR(0.0f), SCALAR(1.0f));
        if (mPaused)
          renderFraction = 1.0f;
      }
    }

    int nObjects = mObjects.size();
    int i;
    for (i = 0 ; i < nObjects ; ++i)
      mObjects[i]->SetRenderPosition(renderFraction);

    CheckGLErrors("After setting render position");

    // Now update all our cameras and render stuff via the viewports
    if (mSelectedIndex >= (int) mObjects.size())
      SelectObject(-1);

    // If we have a character ignore everything else
    if (!mCharacterObjects.empty())
    {
      tObject* charObject = mCharacterObjects[0];
      mCameras[0]->SetPositionObject(charObject);
      if (mCameras[0]->GetOrientationObject() != charObject)
        mCameras[0]->SetOrientationObject(charObject);
      mCameras[0]->SetPositionOffset(tVector3(-7.0f, 0.0f, 4.0f));
    }

    {
      int nCameras = mCameras.size();
      for (i = 0 ; i < nCameras ; ++i)
        mCameras[i]->Update(GetStartOfFrameTime() - GetOldStartOfFrameTime());
      CheckGLErrors("after camera updates");
    }

    // Keep the dragged object under the mouse as the camera moves
    if (mPicking)
      UpdateDrag();

    // Clear
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int nViewports = mViewports.size();
    for (i = 0 ; i < nViewports ; ++i)
    {
      mViewports[i]->Render();
      if (tDebugConfig::mRenderCollisionContacts)
        RenderAllContacts(tDebugConfig::mRenderCollisionContacts);
      // Show the selected object
      tObject * selected = GetSelectedObject();
      if (selected && selected->GetBody())
        selected->RenderWorldBoundingBox(&tVector3::Look()); // look = red
    }
    CheckGLErrors("After viewport renders");

    RenderHelp();
  }

  SDL_GL_SwapWindow(GetMainWindow());

  CheckGLErrors("After buffer swap");
}

//======================================================================================================================
// StartMainLoop
//======================================================================================================================
void tJigTestApp::StartMainLoop()
{
  TRACE_METHOD_ONLY(FRAME_1);

  // Synchronise the physics time
  mPhysics.ResetTime(GetStartOfFrameTime());

  tSDLApplicationBase::StartMainLoop();
}


//======================================================================================================================
// SetupExperiment
//======================================================================================================================
void tJigTestApp::SetupExperiment()
{
  TRACE_METHOD_ONLY(ONCE_1);
  unsigned i;

  // Clear up
  {
    RemoveAllObjects(false);
    for (i = 0 ; i < mCameras.size(); ++i)
      delete mCameras[i];
    mCameras.resize(0);
    for (i = 0 ; i < mViewports.size(); ++i)
      delete mViewports[i];
    mViewports.resize(0);

    delete mCollisionSystem;
    mCollisionSystem = 0;
  }

  switch(tAppConfig::mCollisionSystemType)
  {
  case 0:
    TRACE_FILE_IF(ONCE_1)
      TRACE("Creating tCollisionSystemBrute\n");
    mCollisionSystem = new tCollisionSystemBrute();
    break;
  case 1:
    TRACE_FILE_IF(ONCE_1)
      TRACE("Creating tCollisionSystemGrid\n");
    mCollisionSystem = new tCollisionSystemGrid(32, 32, 4,
                                                tAppConfig::mCollisionGridSize,
                                                tAppConfig::mCollisionGridSize,
                                                tAppConfig::mCollisionGridSize);
    break;
  default:
    TRACE("Collision system type %d not handled\n", tAppConfig::mCollisionSystemType);
    mCollisionSystem = new tCollisionSystemBrute();
    break;
  }
  mPhysics.SetCollisionSystem(mCollisionSystem);

  bool collisionUseSweptTests = false;
  GetConfigFile().GetValue("collision_use_swept_tests", collisionUseSweptTests);
  mCollisionSystem->SetUseSweepTests(collisionUseSweptTests);

  // Create a heightmap or planes for objects to sit on
  string heightmapFilename("none");
  GetConfigFile().GetValue("heightmap_filename", heightmapFilename);
  if (heightmapFilename != string("none"))
  {
    CreateFileHeightmap(heightmapFilename);
  }
  else
  {
    bool useBumpyHeightmap = false;
    GetConfigFile().GetValue("use_bumpy_heightmap", useBumpyHeightmap);
    if (useBumpyHeightmap)
    {
      CreateBumpyHeightmap();
    }
    else
    {
      tScalar planeOffset = -2.0f;
      tScalar planeAngle = 30.0f;
      GetConfigFile().GetValue("plane_offset", planeOffset);
      GetConfigFile().GetValue("plane_angle", planeAngle);
      tScalar planeElasticity = 0.0f;
      tScalar planeStaticFriction = 0.5f;
      tScalar planeDynamicFriction = 0.3f;
      GetConfigFile().GetValue("plane_elasticity", planeElasticity);
      GetConfigFile().GetValue("plane_static_friction", planeStaticFriction);
      GetConfigFile().GetValue("plane_dynamic_friction", planeDynamicFriction);

      tPlaneObject * plane0 = new tPlaneObject(tPlane(tVector3::Up(),
                                                      tVector3(0.0f)));

      tVector3 planeUp = Matrix33Alpha(planeAngle) * tVector3::Up();
      tVector3 offset = planeOffset * tVector3::Up();
      planeUp = Matrix33Gamma(-90.0f) * planeUp;
      tPlaneObject * plane1 = new tPlaneObject(tPlane(planeUp, offset));
      planeUp = Matrix33Gamma(120.0f) * planeUp;
      tPlaneObject * plane2 = new tPlaneObject(tPlane(planeUp, offset));
      planeUp = Matrix33Gamma(120.0f) * planeUp;
      tPlaneObject * plane3 = new tPlaneObject(tPlane(planeUp, offset));

      plane0->SetProperties(planeElasticity,
                            planeStaticFriction,
                            planeDynamicFriction);
      plane1->SetProperties(planeElasticity,
                            planeStaticFriction,
                            planeDynamicFriction);
      plane2->SetProperties(planeElasticity,
                            planeStaticFriction,
                            planeDynamicFriction);
      plane3->SetProperties(planeElasticity,
                            planeStaticFriction,
                            planeDynamicFriction);

      mObjects.push_back(plane0);
      mObjects.push_back(plane1);
      mObjects.push_back(plane2);
      mObjects.push_back(plane3);
      mRenderManager.AddObject(plane0);
      mRenderManager.AddObject(plane1);
      mRenderManager.AddObject(plane2);
      mRenderManager.AddObject(plane3);
      // Need to add non-physical objects to collision
      mCollisionSystem->AddCollisionSkin(plane0->GetCollisionSkin());
      mCollisionSystem->AddCollisionSkin(plane1->GetCollisionSkin());
      mCollisionSystem->AddCollisionSkin(plane2->GetCollisionSkin());
      mCollisionSystem->AddCollisionSkin(plane3->GetCollisionSkin());
    }
  }

  bool doUnequalExp = false;
  GetConfigFile().GetValue("do_unequal_exp", doUnequalExp);
  if (doUnequalExp)
    SetupUnequalExperiment();

  // Create some spheres
  int nSpheres = 1;
  tScalar sphereMinRadius = 1.0f;
  tScalar sphereMaxRadius = 1.0f;
  tScalar sphereElasticity = 0.0f;
  tScalar sphereStaticFriction = 0.5f;
  tScalar sphereDynamicFriction = 0.3f;
  tScalar sphereDensity = 500.0f;
  tScalar sphereX = 0.0f;
  tScalar sphereY = 0.0f;
  tScalar sphereZ = 0.0f;
  GetConfigFile().GetValue("sphere_x", sphereX);
  GetConfigFile().GetValue("sphere_y", sphereY);
  GetConfigFile().GetValue("sphere_z", sphereZ);
  GetConfigFile().GetValue("num_spheres", nSpheres);
  GetConfigFile().GetValue("sphere_min_radius", sphereMinRadius);
  GetConfigFile().GetValue("sphere_max_radius", sphereMaxRadius);
  GetConfigFile().GetValue("sphere_elasticity", sphereElasticity);
  GetConfigFile().GetValue("sphere_static_friction", sphereStaticFriction);
  GetConfigFile().GetValue("sphere_dynamic_friction", sphereDynamicFriction);
  GetConfigFile().GetValue("sphere_density", sphereDensity);

  for (int iSphere = 0 ; iSphere < nSpheres ; ++iSphere)
  {
    tSphereObject * sphereObject =
      new tSphereObject(RangedRandom(sphereMinRadius, sphereMaxRadius));
    sphereObject->SetProperties(sphereElasticity,
                                sphereStaticFriction,
                                sphereDynamicFriction);
    sphereObject->SetDensity(sphereDensity);
    mObjects.push_back(sphereObject);
    sphereObject->GetBody()->MoveTo(
      tVector3(sphereX, sphereY,
               sphereZ + (0.5f + iSphere) * (2.0f * sphereMaxRadius)) +
      0.05f * sphereMaxRadius *
      tVector3(RangedRandom(-1, 1), RangedRandom(-1, 1), 0.0f),
      tMatrix33::Identity());
    mRenderManager.AddObject(sphereObject);
    // Physics will register collision for objects passed to it.
    sphereObject->GetBody()->EnableBody();
  }

  // Create a jenga stack
  int jengaStackHeight = 0;
  tScalar jengaBoxWidth = 1.0f;
  tScalar jengaBoxHeight = 0.5f;
  tScalar jengaStackX = 0.0f;
  tScalar jengaStackY = 0.0f;
  tScalar jengaStackZ = 0.0f;
  GetConfigFile().GetValue("jenga_stack_height", jengaStackHeight);
  GetConfigFile().GetValue("jenga_box_width", jengaBoxWidth);
  GetConfigFile().GetValue("jenga_box_height", jengaBoxHeight);
  GetConfigFile().GetValue("jenga_stack_x", jengaStackX);
  GetConfigFile().GetValue("jenga_stack_y", jengaStackY);
  GetConfigFile().GetValue("jenga_stack_z", jengaStackZ);

  BuildJengaStack(jengaStackHeight,
                  tVector3(jengaBoxWidth * 3.0f,
                           jengaBoxWidth,
                           jengaBoxHeight),
                  tVector3(jengaStackX, jengaStackY, jengaStackZ));

  // Create some boxes
  int nBoxes = 1;
  tScalar boxMinSide = 1.0f;
  tScalar boxMaxSide = 1.0f;
  tScalar boxElasticity = 0.0f;
  tScalar boxStaticFriction = 0.5f;
  tScalar boxDynamicFriction = 0.3f;
  tScalar boxDensity = 500.0f;
  tScalar boxX = 0.0f;
  tScalar boxY = 0.0f;
  tScalar boxZ = 0.0f;
  tScalar boxZSpacing = 0.0f;
  tVector3 boxScale(1, 1, 1);
  GetConfigFile().GetValue("box_x", boxX);
  GetConfigFile().GetValue("box_y", boxY);
  GetConfigFile().GetValue("box_z", boxZ);
  GetConfigFile().GetValue("box_scale_x", boxScale.x);
  GetConfigFile().GetValue("box_scale_y", boxScale.y);
  GetConfigFile().GetValue("box_scale_z", boxScale.z);
  GetConfigFile().GetValue("num_boxes", nBoxes);
  GetConfigFile().GetValue("box_min_side", boxMinSide);
  GetConfigFile().GetValue("box_max_side", boxMaxSide);
  GetConfigFile().GetValue("box_elasticity", boxElasticity);
  GetConfigFile().GetValue("box_static_friction", boxStaticFriction);
  GetConfigFile().GetValue("box_dynamic_friction", boxDynamicFriction);
  GetConfigFile().GetValue("box_density", boxDensity);
  GetConfigFile().GetValue("box_z_spacing", boxZSpacing);

  tScalar z = boxZ - tAppConfig::mAllowedPenetration;
  for (int iBox = 0 ; iBox < nBoxes ; ++iBox)
  {
    tVector3 sides = ElementMult(boxScale, tVector3(RangedRandom(boxMinSide, boxMaxSide),
                                                    RangedRandom(boxMinSide, boxMaxSide),
                                                    RangedRandom(boxMinSide, boxMaxSide)));
    tBoxObject * boxObject = new tBoxObject(sides, false);
    boxObject->SetProperties(boxElasticity,
                             boxStaticFriction,
                             boxDynamicFriction);
    boxObject->SetDensity(boxDensity);
    mObjects.push_back(boxObject);
    boxObject->GetBody()->MoveTo(
      tVector3(boxX, boxY, z + boxZ + 0.5f * sides.z),
      Matrix33Gamma(RangedRandom(0.0f, 0.0f)));
    z += sides.z + boxZSpacing - tAppConfig::mAllowedPenetration;
    mRenderManager.AddObject(boxObject);
    // Physics will register collision for objects passed to it.
    boxObject->GetBody()->EnableBody();
  }

  // Create some capsules
  int numCapsules = 0;
  GetConfigFile().GetValue("num_capsules", numCapsules);
  for (int iCapsule = 0 ; iCapsule < numCapsules ; ++iCapsule)
  {
    tCapsuleObject * capsule = CreateCapsule();
    capsule->GetBody()->MoveTo(capsule->GetBody()->GetPosition() +
                               iCapsule * 1.0f * tVector3::Up(),
                               capsule->GetBody()->GetOrientation());
  }

  // Create some capsules
  int numCompounds = 0;
  GetConfigFile().GetValue("num_compounds", numCompounds);
  for (int iCompound = 0 ; iCompound < numCompounds ; ++iCompound)
  {
    tBoxSphereObject * compound = CreateCompound();
    compound->GetBody()->MoveTo(compound->GetBody()->GetPosition() +
                               iCompound * 1.0f * tVector3::Up(),
                               compound->GetBody()->GetOrientation());
  }


  // Create a wall
  int numWalls = 1;
  tScalar wallSpacing = 10.0f;
  int wallHeight = 0;
  int wallWidth = 0;
  tScalar wallBoxX = 0.5f;
  tScalar wallBoxY = 1.0f;
  tScalar wallBoxZ = 0.5f;
  tScalar wallX = 0.0f;
  tScalar wallY = 0.0f;
  tScalar wallZ = 0.0f;
  bool wallPyramid = false;
  GetConfigFile().GetValue("num_walls", numWalls);
  GetConfigFile().GetValue("wall_spacing", wallSpacing);
  GetConfigFile().GetValue("wall_stack_height", wallHeight);
  GetConfigFile().GetValue("wall_stack_width", wallWidth);
  GetConfigFile().GetValue("wall_box_x", wallBoxX);
  GetConfigFile().GetValue("wall_box_y", wallBoxY);
  GetConfigFile().GetValue("wall_box_z", wallBoxZ);
  GetConfigFile().GetValue("wall_stack_x", wallX);
  GetConfigFile().GetValue("wall_stack_y", wallY);
  GetConfigFile().GetValue("wall_stack_z", wallZ);
  GetConfigFile().GetValue("wall_pyramid", wallPyramid);

  for (int iWall = 0 ; iWall < numWalls ; ++iWall)
  {
    tVector3 offset = -iWall * wallSpacing * tVector3::Look();
    BuildWall(wallHeight, wallWidth,
              tVector3(wallBoxX, wallBoxY, wallBoxZ),
              offset + tVector3(wallX, wallY, wallZ),
              wallPyramid);
  }
  // Create some cars
  int numCars = 0;
  GetConfigFile().GetValue("num_cars", numCars);
  for (int iCar = 0 ; iCar < numCars ; ++iCar)
  {
    tCarObject * car = CreateCar();
    car->GetBody()->MoveTo(car->GetBody()->GetPosition() +
                           iCar * 3.0f * tVector3::Up(),
                           car->GetBody()->GetOrientation());
  }

  // Create a box chain
  int boxChainLen = 0;
  GetConfigFile().GetValue("box_chain_len", boxChainLen);
  if (boxChainLen > 0)
  {
    tScalar boxChainX = 0.0f;
    tScalar boxChainY = 0.0f;
    tScalar boxChainZ = 0.0f;
    tScalar boxChainBoxX = 1.0f;
    tScalar boxChainBoxY = 2.0f;
    tScalar boxChainBoxZ = 1.0f;
    tScalar boxChainDensity = 100.0f;
    GetConfigFile().GetValue("box_chain_x", boxChainX);
    GetConfigFile().GetValue("box_chain_y", boxChainY);
    GetConfigFile().GetValue("box_chain_z", boxChainZ);
    GetConfigFile().GetValue("box_chain_box_x", boxChainBoxX);
    GetConfigFile().GetValue("box_chain_box_y", boxChainBoxY);
    GetConfigFile().GetValue("box_chain_box_z", boxChainBoxZ);
    GetConfigFile().GetValue("box_chain_density", boxChainDensity);
    tBoxChain * chain = new tBoxChain(
      &mRenderManager,
      boxChainLen,
      tVector3(boxChainX, boxChainY, boxChainZ),
      tVector3(boxChainBoxX, boxChainBoxY, boxChainBoxZ),
      boxChainDensity,
      tBoxChain::CHAIN_HINGE);
    mObjects.push_back(chain);
  }

  // Create a sphere chain
  int sphereChainLen = 0;
  GetConfigFile().GetValue("sphere_chain_len", sphereChainLen);
  if (sphereChainLen > 0)
  {
    tScalar sphereChainX = 0.0f;
    tScalar sphereChainY = 0.0f;
    tScalar sphereChainZ = 0.0f;
    tScalar sphereChainRadius = 1.0f;
    tScalar sphereChainDensity = 100.0f;
    GetConfigFile().GetValue("sphere_chain_x", sphereChainX);
    GetConfigFile().GetValue("sphere_chain_y", sphereChainY);
    GetConfigFile().GetValue("sphere_chain_z", sphereChainZ);
    GetConfigFile().GetValue("sphere_chain_radius", sphereChainRadius);
    GetConfigFile().GetValue("sphere_chain_density", sphereChainDensity);
    tSphereChain * chain = new tSphereChain(
      &mRenderManager,
      sphereChainLen,
      tVector3(sphereChainX, sphereChainY, sphereChainZ),
      sphereChainRadius,
      sphereChainDensity,
      tSphereChain::CHAIN_BALLNSOCKET);
    mObjects.push_back(chain);
  }

  // Create some ragdolls
  int numRagdolls = 0;
  GetConfigFile().GetValue("num_ragdolls", numRagdolls);
  bool ragdollStartAsleep = false;
  GetConfigFile().GetValue("ragdoll_start_asleep", ragdollStartAsleep);
  for (int iRagdoll = 0 ; iRagdoll < numRagdolls ; ++iRagdoll)
  {
    CreateRagdoll();
  }

  // Create some newton cradles
  int numNewtons = 0;
  GetConfigFile().GetValue("num_newtons", numNewtons);
  for (int iNewton = 0 ; iNewton < numNewtons ; ++iNewton)
  {
    CreateNewton();
  }

  if (false) // Test for PAL problems
  {
    std::vector<tVector3> verts(5);
    verts[0].Set(1, 0, 2);
    verts[1].Set(0, -1, 2);
    verts[2].Set(0, 0, 1);
    verts[3].Set(0, 1, 2);
    verts[4].Set(-1, 0, 2);

    std::vector<tTriangleVertexIndices> inds(4);
    inds[0].Set(2, 1, 0);
    inds[1].Set(2, 0, 3);
    inds[2].Set(1, 2, 4);
    inds[3].Set(2, 3, 4);

    tScalar minCellSize = 1.0f;
    tScalar meshElasticity = 0.0f;
    tScalar meshStaticFriction = 1.0f;
    tScalar meshDynamicFriction = 1.0f;
    int maxTrianglesPerCell = 4;

    tMeshObject* meshObject = new tMeshObject(verts, inds, maxTrianglesPerCell, minCellSize);
    meshObject->SetProperties(meshElasticity,
      meshStaticFriction,
      meshDynamicFriction);
    mObjects.push_back(meshObject);
    mRenderManager.AddObject(meshObject);
    // Need to add non-physical objects to collision
    mCollisionSystem->AddCollisionSkin(meshObject->GetCollisionSkin());
  }


  // mesh test - TODO: properly
  std::string meshName("none");
  GetConfigFile().GetValue("mesh_name", meshName);
  if (meshName != "none")
  {
    tScalar meshScale = 1.0f;
    tVector3 meshOffset(0.0f);
    int maxTrianglesPerCell = 4;
    tScalar minCellSize = 1.0f;
    tScalar meshElasticity = 0.0f;
    tScalar meshStaticFriction = 0.5f;
    tScalar meshDynamicFriction = 0.3f;
    GetConfigFile().GetValue("mesh_scale", meshScale);
    GetConfigFile().GetValue("mesh_x", meshOffset.x);
    GetConfigFile().GetValue("mesh_y", meshOffset.y);
    GetConfigFile().GetValue("mesh_z", meshOffset.z);
    GetConfigFile().GetValue("mesh_max_triangles_per_cell", maxTrianglesPerCell);
    GetConfigFile().GetValue("mesh_min_cell_size", minCellSize);
    GetConfigFile().GetValue("mesh_elasticity",
                             meshElasticity);
    GetConfigFile().GetValue("mesh_static_friction",
                             meshStaticFriction);
    GetConfigFile().GetValue("mesh_dynamic_friction",
                             meshDynamicFriction);

    tMeshObject* meshObject = new tMeshObject(meshName.c_str(), meshScale, maxTrianglesPerCell, minCellSize);
    meshObject->SetProperties(meshElasticity,
                              meshStaticFriction,
                              meshDynamicFriction);
    mObjects.push_back(meshObject);
    mRenderManager.AddObject(meshObject);
    // Need to add non-physical objects to collision
    mCollisionSystem->AddCollisionSkin(meshObject->GetCollisionSkin());
  }

  // Create some viewports and cameras
  int numViews = 1;
  GetConfigFile().GetValue("num_views", numViews);
  if (numViews > 2) numViews = 2;
  if (numViews < 1) numViews = 1;
  if (mObjects.size() < 2)
    numViews = 1;

  tCamera * camera0 = new tCamera;
  mCameras.push_back(camera0);
  tScalar h = numViews == 1 ? 1.0f : 0.5f;
  tViewport * viewport0 = new tViewport(&mRenderManager,
                                        camera0,
                                        0.0f, 0.0f,
                                        h, 1.0f);
  mViewports.push_back(viewport0);

  // Attach the camera to our object
  camera0->SetPosition(tVector3(-20.0f, 1.0f, 15.0f));
  camera0->SetTarget(tVector3::Zero());
  camera0->SetUp(tVector3::Up());
  SetCameraDirection(tVector3::Zero() - camera0->GetPosition());
  mOrbitPoint = tVector3::Zero();

  // If there's a car, select it so that the arrow keys drive it
  if (!mCarObjects.empty())
  {
    for (int i = 0 ; i < (int) mObjects.size() ; ++i)
    {
      if (mObjects[i] == mCarObjects[0])
        SelectObject(i);
    }
  }

  if (numViews == 2)
  {
    tCamera * camera1 = new tCamera;
    mCameras.push_back(camera1);
    tViewport * viewport1 = new tViewport(&mRenderManager,
                                          camera1,
                                          0.5f, 0.0f,
                                          0.5f, 1.0f);
    mViewports.push_back(viewport1);

    // Attach the camera to our object
    camera1->SetPosition(tVector3(-20.0f, 1.0f, 15.0f));
    camera1->SetTarget(tVector3::Zero());
    camera1->SetUp(tVector3::Up());

    camera1->SetTargetObject(mObjects[mObjects.size() - 1]);
  }
}

//======================================================================================================================
// LoadAppConfig
//======================================================================================================================
void tJigTestApp::LoadAppConfig()
{
  TRACE_METHOD_ONLY(ONCE_1);

  GetConfigFile().GetValue("collision_system_type", tAppConfig::mCollisionSystemType);
  GetConfigFile().GetValue("collision_grid_size", tAppConfig::mCollisionGridSize);
  GetConfigFile().GetValue("physics_freq", tAppConfig::mPhysicsFrequency);
  GetConfigFile().GetValue("physics_num_collision_iterations",
                           tAppConfig::mNumCollisionIterations);
  GetConfigFile().GetValue("penetration_relaxation_timesteps",
                           tAppConfig::mNumPenetrationRelaxationTimesteps);
  GetConfigFile().GetValue("physics_num_contact_iterations",
                           tAppConfig::mNumContactIterations);
  GetConfigFile().GetValue("allowed_penetration", tAppConfig::mAllowedPenetration);
  GetConfigFile().GetValue("physics_do_shock_step",
                           tAppConfig::mDoShockStep);
  GetConfigFile().GetValue("physics_coll_toll", tAppConfig::mPhysicsCollToll);
  GetConfigFile().GetValue("physics_gravity_x", tAppConfig::mGravity.x);
  GetConfigFile().GetValue("physics_gravity_y", tAppConfig::mGravity.y);
  GetConfigFile().GetValue("physics_gravity_z", tAppConfig::mGravity.z);
  GetConfigFile().GetValue("physics_enable_freezing", tAppConfig::mEnableFreezing);
  GetConfigFile().GetValue("start_paused", mPaused);
  GetConfigFile().GetValue("max_physics_steps_per_frame",
                           tAppConfig::mMaxPhysicsStepsPerFrame);
  GetConfigFile().GetValue("null_physics_update",
                           tAppConfig::mNullPhysicsUpdate);
  GetConfigFile().GetValue("physics_solver_type", tAppConfig::mSolverType);
  GetConfigFile().GetValue("movie_interval", tAppConfig::mMovieInterval);

  GetConfigFile().GetValue("indicate_frozen_objects",
                           tDebugConfig::mIndicateFrozenObjects);
  GetConfigFile().GetValue("render_collision_bounding_boxes",
                           tDebugConfig::mRenderCollisionBoundingBoxes);
  GetConfigFile().GetValue("render_collision_contacts",
                           tDebugConfig::mRenderCollisionContacts);

  GetConfigFile().GetValue("render_enable", tAppConfig::mRenderEnable);
  GetConfigFile().GetValue("texture_level", tAppConfig::mTextureLevel);
  GetConfigFile().GetValue("render_interpolate", tAppConfig::mRenderInterpolate);
  GetConfigFile().GetValue("cam_speed", tAppConfig::mCamSpeed);
  GetConfigFile().GetValue("object_controller_speed", tAppConfig::mObjectControllerSpeed);
  GetConfigFile().GetValue("projectile_speed", tAppConfig::mProjectileSpeed);

  GetConfigFile().GetValue("physics_vel_threshold",
                           tAppConfig::mPhysicsVelThreshold);
  GetConfigFile().GetValue("physics_ang_vel_threshold",
                           tAppConfig::mPhysicsAngVelThreshold);
  GetConfigFile().GetValue("physics_pos_threshold",
                           tAppConfig::mPhysicsPosThreshold);
  GetConfigFile().GetValue("physics_orient_threshold",
                           tAppConfig::mPhysicsOrientThreshold);
  GetConfigFile().GetValue("physics_deactivation_time",
                           tAppConfig::mPhysicsDeactivationTime);

  GetConfigFile().GetValue("physics_quit_iterations",
                           mQuitIterations);

  GetConfigFile().GetValue("show_help", mShowHelp);

  GetConfigFile().GetValue("skybox",
                           tAppConfig::mSkybox);

  Limit(tAppConfig::mPhysicsFrequency, SCALAR(0.1), SCALAR(10000.0f));

  mPhysics.SetGravity(tAppConfig::mGravity);
  mPhysics.SetNumCollisionIterations(tAppConfig::mNumCollisionIterations);
  mPhysics.SetNumContactIterations(tAppConfig::mNumContactIterations);
  mPhysics.SetNumPenetrationRelaxationTimesteps(tAppConfig::mNumPenetrationRelaxationTimesteps);
  mPhysics.SetAllowedPenetration(tAppConfig::mAllowedPenetration);
  mPhysics.SetCollToll(tAppConfig::mPhysicsCollToll);
  mPhysics.SetDoShockStep(tAppConfig::mDoShockStep);
  mPhysics.EnableFreezing(tAppConfig::mEnableFreezing);
  mPhysics.SetNullUpdate(tAppConfig::mNullPhysicsUpdate);
  if (tAppConfig::mSolverType == "fast")
    mPhysics.SetSolverType(tPhysicsSystem::SOLVER_FAST);
  else if (tAppConfig::mSolverType == "normal")
    mPhysics.SetSolverType(tPhysicsSystem::SOLVER_NORMAL);
  else if (tAppConfig::mSolverType == "combined")
    mPhysics.SetSolverType(tPhysicsSystem::SOLVER_COMBINED);
  else if (tAppConfig::mSolverType == "accumulated")
    mPhysics.SetSolverType(tPhysicsSystem::SOLVER_ACCUMULATED);

  if (tAppConfig::mTextureLevel < 0)
    mRenderManager.SetWireFrameMode(true);

  delete mSkybox;
  mSkybox = new tSkybox(tAppConfig::mSkybox);
  mRenderManager.AddObject(mSkybox);
}


//======================================================================================================================
// Initialise
//======================================================================================================================
bool tJigTestApp::Initialise(const std::string app_name)
{
  TRACE_METHOD_ONLY(ONCE_1);

  bool retval = tSDLApplicationBase::Initialise(app_name);

  LoadAppConfig();

  SetupExperiment();

  SetSleepingParams();

  return retval;
}

//======================================================================================================================
// SetSleepingParams
//======================================================================================================================
void tJigTestApp::SetSleepingParams()
{
  TRACE_METHOD_ONLY(ONCE_3);
  for (unsigned i = 0 ; i < mObjects.size() ; ++i)
  {
    if (mObjects[i]->GetBody())
    {
      mObjects[i]->GetBody()->SetActivityThreshold(
        tAppConfig::mPhysicsVelThreshold,
        tAppConfig::mPhysicsAngVelThreshold);
      mObjects[i]->GetBody()->SetDeactivationThreshold(
        tAppConfig::mPhysicsPosThreshold,
        tAppConfig::mPhysicsOrientThreshold);
      mObjects[i]->GetBody()->SetDeactivationTime(
        tAppConfig::mPhysicsDeactivationTime);
    }
  }
}


//======================================================================================================================
// CreateSphere
//======================================================================================================================
tSphereObject * tJigTestApp::CreateSphere()
{
  TRACE_METHOD_ONLY(ONCE_2);
  tScalar sphereMinRadius = 1.0f;
  tScalar sphereMaxRadius = 1.0f;
  tScalar sphereElasticity = 0.0f;
  tScalar sphereStaticFriction = 0.5f;
  tScalar sphereDynamicFriction = 0.3f;
  tScalar sphereDensity = 500.0f;
  tScalar sphereX = 0.0f;
  tScalar sphereY = 0.0f;
  tScalar sphereZ = 0.0f;
  GetConfigFile().GetValue("sphere_x", sphereX);
  GetConfigFile().GetValue("sphere_y", sphereY);
  GetConfigFile().GetValue("sphere_z", sphereZ);
  GetConfigFile().GetValue("sphere_min_radius", sphereMinRadius);
  GetConfigFile().GetValue("sphere_max_radius", sphereMaxRadius);
  GetConfigFile().GetValue("sphere_elasticity", sphereElasticity);
  GetConfigFile().GetValue("sphere_static_friction", sphereStaticFriction);
  GetConfigFile().GetValue("sphere_dynamic_friction", sphereDynamicFriction);
  GetConfigFile().GetValue("sphere_density", sphereDensity);

  tSphereObject * sphereObject =
    new tSphereObject(RangedRandom(sphereMinRadius, sphereMaxRadius));
  sphereObject->SetProperties(sphereElasticity,
                              sphereStaticFriction,
                              sphereDynamicFriction);
  sphereObject->SetDensity(sphereDensity);
  mObjects.push_back(sphereObject);
  sphereObject->GetBody()->MoveTo(
    tVector3(sphereX, sphereY, sphereZ + 0.5f * 2.0f * sphereMaxRadius),
    tMatrix33::Identity());
  mRenderManager.AddObject(sphereObject);
  sphereObject->GetBody()->EnableBody();
  return sphereObject;
}


//======================================================================================================================
// CreateCharacter
//======================================================================================================================
tCharacterObject * tJigTestApp::CreateCharacter()
{
  TRACE_METHOD_ONLY(ONCE_2);
  tScalar radius = 0.5f;
  tScalar height = 2.0f;
  tCharacterObject * characterObject =
    new tCharacterObject(radius, height);

  mObjects.push_back(characterObject);
  mCharacterObjects.push_back(characterObject);

  characterObject->GetBody()->MoveTo(
    tVector3(0.0f, 0.0f, 0.0f),
    tMatrix33::Identity());
  mRenderManager.AddObject(characterObject);
  characterObject->GetBody()->EnableBody();
  return characterObject;
}

//======================================================================================================================
// CreateCapsule
//======================================================================================================================
tCapsuleObject * tJigTestApp::CreateCapsule()
{
  TRACE_METHOD_ONLY(ONCE_2);
  tScalar capsuleMinRadius = 1.0f;
  tScalar capsuleMaxRadius = 1.0f;
  tScalar capsuleMinLength = 2.0f;
  tScalar capsuleMaxLength = 2.0f;
  tScalar capsuleElasticity = 0.0f;
  tScalar capsuleStaticFriction = 0.5f;
  tScalar capsuleDynamicFriction = 0.3f;
  tScalar capsuleDensity = 500.0f;
  tScalar capsuleX = 0.0f;
  tScalar capsuleY = 0.0f;
  tScalar capsuleZ = 0.0f;
  GetConfigFile().GetValue("capsule_x", capsuleX);
  GetConfigFile().GetValue("capsule_y", capsuleY);
  GetConfigFile().GetValue("capsule_z", capsuleZ);
  GetConfigFile().GetValue("capsule_min_radius", capsuleMinRadius);
  GetConfigFile().GetValue("capsule_max_radius", capsuleMaxRadius);
  GetConfigFile().GetValue("capsule_min_length", capsuleMinLength);
  GetConfigFile().GetValue("capsule_max_length", capsuleMaxLength);
  GetConfigFile().GetValue("capsule_elasticity", capsuleElasticity);
  GetConfigFile().GetValue("capsule_static_friction", capsuleStaticFriction);
  GetConfigFile().GetValue("capsule_dynamic_friction", capsuleDynamicFriction);
  GetConfigFile().GetValue("capsule_density", capsuleDensity);

  tCapsuleObject * capsuleObject =
    new tCapsuleObject(RangedRandom(capsuleMinRadius, capsuleMaxRadius),
                       RangedRandom(capsuleMinLength, capsuleMaxLength));
  capsuleObject->SetProperties(capsuleElasticity,
                               capsuleStaticFriction,
                               capsuleDynamicFriction);
  capsuleObject->SetDensity(capsuleDensity);
  mObjects.push_back(capsuleObject);
  capsuleObject->GetBody()->MoveTo(
    tVector3(capsuleX, capsuleY, capsuleZ + 0.5f * 2.0f * capsuleMaxRadius),
    Matrix33Gamma(RangedRandom(0.0f, 360.0f)));
  mRenderManager.AddObject(capsuleObject);
  capsuleObject->GetBody()->EnableBody();
  return capsuleObject;
}

//======================================================================================================================
// CreateBox
//======================================================================================================================
tBoxObject * tJigTestApp::CreateBox()
{
  TRACE_METHOD_ONLY(ONCE_2);
  tScalar boxMinSide = 1.0f;
  tScalar boxMaxSide = 1.0f;
  tScalar boxElasticity = 0.0f;
  tScalar boxStaticFriction = 0.5f;
  tScalar boxDynamicFriction = 0.3f;
  tScalar boxDensity = 500.0f;
  tScalar boxX = 0.0f;
  tScalar boxY = 0.0f;
  tScalar boxZ = 0.0f;
  tVector3 boxScale(1, 1, 1);
  GetConfigFile().GetValue("box_x", boxX);
  GetConfigFile().GetValue("box_y", boxY);
  GetConfigFile().GetValue("box_z", boxZ);
  GetConfigFile().GetValue("box_scale_x", boxScale.x);
  GetConfigFile().GetValue("box_scale_y", boxScale.y);
  GetConfigFile().GetValue("box_scale_z", boxScale.z);
  GetConfigFile().GetValue("box_min_side", boxMinSide);
  GetConfigFile().GetValue("box_max_side", boxMaxSide);
  GetConfigFile().GetValue("box_elasticity", boxElasticity);
  GetConfigFile().GetValue("box_static_friction", boxStaticFriction);
  GetConfigFile().GetValue("box_dynamic_friction", boxDynamicFriction);
  GetConfigFile().GetValue("box_density", boxDensity);

  tBoxObject * boxObject = new tBoxObject(
    ElementMult(boxScale, tVector3(RangedRandom(boxMinSide, boxMaxSide),
                                   RangedRandom(boxMinSide, boxMaxSide),
                                   RangedRandom(boxMinSide, boxMaxSide))), true);
  boxObject->SetProperties(boxElasticity,
                           boxStaticFriction,
                           boxDynamicFriction);
  boxObject->SetDensity(boxDensity);
  mObjects.push_back(boxObject);
  boxObject->GetBody()->MoveTo(
    tVector3(boxX, boxY, boxZ + 0.5f * boxMaxSide),
    Matrix33Gamma(RangedRandom(0.0f, 360.0f)));
  mRenderManager.AddObject(boxObject);
  // Physics will register collision for objects passed to it.
  boxObject->GetBody()->EnableBody();
  return boxObject;
}

//======================================================================================================================
// CreateCompound
//======================================================================================================================
tBoxSphereObject * tJigTestApp::CreateCompound()
{
  TRACE_METHOD_ONLY(ONCE_2);
  tScalar compoundMinSide = 1.0f;
  tScalar compoundMaxSide = 1.0f;
  tScalar compoundElasticity = 0.0f;
  tScalar compoundStaticFriction = 0.5f;
  tScalar compoundDynamicFriction = 0.3f;
  tScalar compoundDensity = 500.0f;
  tScalar compoundX = 0.0f;
  tScalar compoundY = 0.0f;
  tScalar compoundZ = 0.0f;
  tVector3 compoundScale(1, 1, 1);
  GetConfigFile().GetValue("compound_x", compoundX);
  GetConfigFile().GetValue("compound_y", compoundY);
  GetConfigFile().GetValue("compound_z", compoundZ);
  GetConfigFile().GetValue("compound_scale_x", compoundScale.x);
  GetConfigFile().GetValue("compound_scale_y", compoundScale.y);
  GetConfigFile().GetValue("compound_scale_z", compoundScale.z);
  GetConfigFile().GetValue("compound_min_side", compoundMinSide);
  GetConfigFile().GetValue("compound_max_side", compoundMaxSide);
  GetConfigFile().GetValue("compound_elasticity", compoundElasticity);
  GetConfigFile().GetValue("compound_static_friction", compoundStaticFriction);
  GetConfigFile().GetValue("compound_dynamic_friction", compoundDynamicFriction);
  GetConfigFile().GetValue("compound_density", compoundDensity);

  tBoxSphereObject * compoundObject = new tBoxSphereObject(
    ElementMult(compoundScale, tVector3(RangedRandom(compoundMinSide, compoundMaxSide),
                                   RangedRandom(compoundMinSide, compoundMaxSide),
                                   RangedRandom(compoundMinSide, compoundMaxSide))), true);
  compoundObject->SetProperties(compoundElasticity,
                           compoundStaticFriction,
                           compoundDynamicFriction);
  compoundObject->SetDensity(compoundDensity);
  mObjects.push_back(compoundObject);
  compoundObject->GetBody()->MoveTo(
    tVector3(compoundX, compoundY, compoundZ + 0.5f * compoundMaxSide),
    Matrix33Gamma(RangedRandom(0.0f, 360.0f)));
  mRenderManager.AddObject(compoundObject);
  // Physics will register collision for objects passed to it.
  compoundObject->GetBody()->EnableBody();
  return compoundObject;
}


//======================================================================================================================
// SetupUnequalExperiment
//======================================================================================================================
void tJigTestApp::SetupUnequalExperiment()
{
  TRACE_METHOD_ONLY(ONCE_1);
  tScalar boxElasticity = 0.0f;
  tScalar boxStaticFriction = 0.5f;
  tScalar boxDynamicFriction = 0.3f;
  GetConfigFile().GetValue("box_elasticity", boxElasticity);
  GetConfigFile().GetValue("box_static_friction", boxStaticFriction);
  GetConfigFile().GetValue("box_dynamic_friction", boxDynamicFriction);

  tBoxObject * boxObject = new tBoxObject(tVector3(0.1f, 12.0f, 6.0f), true);
  boxObject->SetProperties(boxElasticity,
                           boxStaticFriction,
                           boxDynamicFriction);
  boxObject->SetDensity(100.0f);
  mObjects.push_back(boxObject);
  boxObject->GetBody()->MoveTo(tVector3(0.0f, 0.0f, 3.0f),
                               tMatrix33::Identity());
  mRenderManager.AddObject(boxObject);
  boxObject->GetBody()->EnableBody();

  boxObject = new tBoxObject(tVector3(4.0f, 5.0f, 2.0f), true);
  boxObject->SetProperties(boxElasticity,
                           boxStaticFriction,
                           boxDynamicFriction);
  boxObject->SetDensity(100.0f);
  mObjects.push_back(boxObject);
  boxObject->GetBody()->MoveTo(tVector3(0.0f, 0.0f, 10.0f),
                               tMatrix33::Identity());
  mRenderManager.AddObject(boxObject);
  boxObject->GetBody()->EnableBody();
}

//======================================================================================================================
// BuildJengaStack
//======================================================================================================================
void tJigTestApp::BuildJengaStack(int height,
                                  const tVector3 & boxDims,
                                  const tVector3 & pos)
{
  TRACE_METHOD_ONLY(ONCE_1);
  tScalar boxElasticity = 0.0f;
  tScalar boxStaticFriction = 0.5f;
  tScalar boxDynamicFriction = 0.3f;
  tScalar boxDensity = 500.0f;
  GetConfigFile().GetValue("box_elasticity", boxElasticity);
  GetConfigFile().GetValue("box_static_friction", boxStaticFriction);
  GetConfigFile().GetValue("box_dynamic_friction", boxDynamicFriction);
  GetConfigFile().GetValue("box_density", boxDensity);

  for (int iH = 0 ; iH < height ; ++iH)
  {
    tVector3 thisPos(pos);
    thisPos.z += (iH + 0.5f) * (boxDims.z - tAppConfig::mAllowedPenetration) - tAppConfig::mAllowedPenetration;
    tVector3 leftDir = tVector3::Left();
    tVector3 fwdDir = tVector3::Look();
    tMatrix33 rot(tMatrix33::IDENTITY);
    if (iH % 2)
    {
      rot = Matrix33Gamma(90.0f);
      leftDir = rot * leftDir;
      fwdDir = rot * fwdDir;
    }

    for (int iBox = -1 ; iBox <= 1 ; ++iBox)
    {
      tBoxObject * boxObject = new tBoxObject(boxDims, false);
      boxObject->SetProperties(boxElasticity,
                               boxStaticFriction,
                               boxDynamicFriction);
      boxObject->SetDensity(boxDensity);
      mObjects.push_back(boxObject);
      boxObject->GetBody()->MoveTo(
        thisPos + (tScalar) iBox * leftDir * boxDims.y,
        rot);
      mRenderManager.AddObject(boxObject);
      // Physics will register collision for objects passed to it.
      boxObject->GetBody()->EnableBody();
    }
  }
}

//======================================================================================================================
// BuildJengaStack
//======================================================================================================================
void tJigTestApp::BuildWall(int height,
                            int width,
                            const tVector3 & boxDims,
                            const tVector3 & pos,
                            bool pyramid)
{
  TRACE_METHOD_ONLY(ONCE_1);
  tScalar boxElasticity = 0.0f;
  tScalar boxStaticFriction = 0.5f;
  tScalar boxDynamicFriction = 0.3f;
  tScalar boxDensity = 500.0f;
  GetConfigFile().GetValue("box_elasticity", boxElasticity);
  GetConfigFile().GetValue("box_static_friction", boxStaticFriction);
  GetConfigFile().GetValue("box_dynamic_friction", boxDynamicFriction);
  GetConfigFile().GetValue("box_density", boxDensity);

  for (int iH = 0 ; iH < height ; ++iH)
  {
    for (int iW = 0 ; iW < width ; ++iW)
    {
      tVector3 thisPos(pos);
      thisPos.z += (iH + 0.5f) * (boxDims.z - tAppConfig::mAllowedPenetration) - tAppConfig::mAllowedPenetration;
      thisPos.y += iW * boxDims.y - 0.5f * width;
      if (iH % 2)
      {
        if (iW == width-1)
          continue;
        thisPos.y += 0.5f * boxDims.y;
      }

      if (pyramid)
      {
        int iH2 = iH / 2;
        if (iW < iH2)
          continue;
        int maxW = width - (iH2 + 1);
        if (iH % 2)
          maxW -= 1;
        if (iW > maxW)
          continue;
      }
      else
      {
        if ( (iH == height - 1) &&
             (iH % 2 == 0) &&
             ( (iW == 0) || (iW == width - 1) ) )
          continue;
      }
      tBoxObject * boxObject = new tBoxObject(boxDims, false);
      boxObject->SetProperties(boxElasticity,
                               boxStaticFriction,
                               boxDynamicFriction);
      boxObject->SetDensity(boxDensity);
      mObjects.push_back(boxObject);
      boxObject->GetBody()->MoveTo(thisPos, tMatrix33::Identity());
      mRenderManager.AddObject(boxObject);
      // Physics will register collision for objects passed to it.
      boxObject->GetBody()->EnableBody();
    }
  }
}

//======================================================================================================================
// RemoveAllObjects
//======================================================================================================================
void tJigTestApp::RemoveAllObjects(bool onlyPhysical)
{
  TRACE_METHOD_ONLY(ONCE_1);

  mObjectController.DisableConstraint();
  StopDrag();
  mSelectedIndex = -1;

  unsigned i;
  unsigned nCameras = mCameras.size();
  for (i = 0 ; i < nCameras ; ++i)
  {
    mCameras[i]->SetTargetObject(0);
    mCameras[i]->SetOrientationObject(0);
    mCameras[i]->SetPositionObject(0);
    mCameras[i]->SetTrackTarget(0, 0, 0, 0);
  }
  for (i = 0 ; i < mObjects.size() ; ++i)
  {
    if (!onlyPhysical || mObjects[i]->GetBody())
    {
      TRACE("Deleting object %d\n", i);
      delete mObjects[i];
      mObjects[i] = 0;
      TRACE("Done deleting object %d\n", i);
    }
  }
  tObjects::iterator pos = remove(mObjects.begin(),
                                  mObjects.end(),
                                  (tObject *) 0);
  mObjects.erase(pos, mObjects.end());

  mCarAI.RemoveAllCars();
  mCarObjects.resize(0);
  mCharacterObjects.resize(0);
}

//======================================================================================================================
// RestartExperiment
//======================================================================================================================
void tJigTestApp::RestartExperiment()
{
  TRACE_METHOD_ONLY(ONCE_1);
  SetupExperiment();
}

//======================================================================================================================
// CreateBumpyHeightmap
//======================================================================================================================
void tJigTestApp::CreateBumpyHeightmap()
{
  TRACE_METHOD_ONLY(ONCE_1);

  int nx = 128;
  int ny = 128;
  tScalar dx = 1.0f;
  tScalar dy = 1.0f;
  tScalar x0 = 0.0f;
  tScalar y0 = 0.0f;
  tScalar heightmapElasticity = 0.0f;
  tScalar heightmapStaticFriction = 0.5f;
  tScalar heightmapDynamicFriction = 0.3f;
  GetConfigFile().GetValue("heightmap_nx", nx);
  GetConfigFile().GetValue("heightmap_ny", ny);
  GetConfigFile().GetValue("heightmap_dx", dx);
  GetConfigFile().GetValue("heightmap_dy", dy);
  GetConfigFile().GetValue("heightmap_x0", x0);
  GetConfigFile().GetValue("heightmap_y0", y0);
  GetConfigFile().GetValue("heightmap_elasticity",
                           heightmapElasticity);
  GetConfigFile().GetValue("heightmap_static_friction",
                           heightmapStaticFriction);
  GetConfigFile().GetValue("heightmap_dynamic_friction",
                           heightmapDynamicFriction);

  // Create the bumps
  tArray2D<tScalar> heights(nx, ny, 0.0f);

  tScalar lambda = nx * (dx / 5.0f);

  tScalar peakRadius = lambda * 2.0f;
  tScalar peakRadiusDelta = lambda;

  tScalar xMin = x0 - 0.5f * (nx - 1) * dx;
  tScalar yMin = y0 - 0.5f * (ny - 1) * dy;

  int i, j;
  for (i = 0 ; i < nx ; ++i)
  {
    for (j = 0 ; j < ny ; ++j)
    {
      tScalar x = xMin + i * dx ;
      tScalar y = yMin + j * dy;
      tScalar radius = Hypot(x, y);
      tScalar scale = Exp(-Sq(radius - peakRadius) /
                          Sq(peakRadiusDelta));
      tScalar z = 0.2f * scale * lambda *
        SinDeg(360.0f * x / lambda) *
        SinDeg(360.0f * y / lambda);
      heights(i, j) = z;
    }
  }

  // Make a bumpy bit
  for (i = 0 ; i < nx ; ++i)
  {
    for (j = 0 ; j < ny ; ++j)
    {
      tScalar x = xMin + i * dx ;
      tScalar y = yMin + j * dy;
      tScalar radius = Hypot(x - lambda, y - lambda);
      if (radius < lambda)
        heights(i, j) += RangedRandom(-0.1f, 0.1f);
    }
  }

  // Make sure the middle is completely flat
  tScalar rMin = 50.0f;
  tScalar rMax = 70.0f;
  for (i = 0 ; i < nx ; ++i)
  {
    for (j = 0 ; j < ny ; ++j)
    {
      tScalar x = xMin + i * dx ;
      tScalar y = yMin + j * dy;
      tScalar radius = Hypot(x, y);
      tScalar frac = (radius - rMin) / (rMax - rMin);
      Limit(frac, SCALAR(0.0f), SCALAR(1.0f));
      if (radius < rMax)
      {
        heights(i, j) = heights(i, j) * frac;
      }
    }
  }

  // Create the actual objects
  tHeightmap heightmap(heights, x0, y0, dx, dy);
  tHeightmapObject * heightmapObject = new tHeightmapObject(heightmap);
  heightmapObject->SetProperties(heightmapElasticity,
                                 heightmapStaticFriction,
                                 heightmapDynamicFriction);
  mObjects.push_back(heightmapObject);
  mRenderManager.AddObject(heightmapObject);

  // Need to add non-physical objects to collision
  mCollisionSystem->AddCollisionSkin(heightmapObject->GetCollisionSkin());
}

//======================================================================================================================
// CreateFileHeightmap
//======================================================================================================================
void tJigTestApp::CreateFileHeightmap(const string & fileName)
{
  TRACE_METHOD_ONLY(ONCE_1);

  int nx = 128;
  int ny = 128;
  tScalar dx = 1.0f;
  tScalar dy = 1.0f;
  tScalar zMin = 0.0f;
  tScalar zMax = 0.0f;
  tScalar heightmapElasticity = 0.0f;
  tScalar heightmapStaticFriction = 0.5f;
  tScalar heightmapDynamicFriction = 0.3f;
  GetConfigFile().GetValue("heightmap_nx", nx);
  GetConfigFile().GetValue("heightmap_ny", ny);
  GetConfigFile().GetValue("heightmap_dx", dx);
  GetConfigFile().GetValue("heightmap_dy", dy);
  GetConfigFile().GetValue("heightmap_zmin", zMin);
  GetConfigFile().GetValue("heightmap_zmax", zMax);
  GetConfigFile().GetValue("heightmap_elasticity",
                           heightmapElasticity);
  GetConfigFile().GetValue("heightmap_static_friction",
                           heightmapStaticFriction);
  GetConfigFile().GetValue("heightmap_dynamic_friction",
                           heightmapDynamicFriction);

  // Create the actual objects
  tHeightmapObject * heightmapObject =
    new tHeightmapObject(fileName, dx, dy, zMin, zMax);
  heightmapObject->SetProperties(heightmapElasticity,
                                 heightmapStaticFriction,
                                 heightmapDynamicFriction);
  mObjects.push_back(heightmapObject);
  mRenderManager.AddObject(heightmapObject);
  // Need to add non-physical objects to collision
  mCollisionSystem->AddCollisionSkin(heightmapObject->GetCollisionSkin());
}


//======================================================================================================================
// CreateCar
//======================================================================================================================
tCarObject * tJigTestApp::CreateCar()
{
  tScalar carMinX = -2.0f;
  tScalar carMinY = -1.0f;
  tScalar carMinZ = 0.0f;
  tScalar carMaxX = 2.0f;
  tScalar carMaxY = 1.0f;
  tScalar carMaxZ = 1.5f;
  tScalar carElasticity = 0.0f;
  tScalar carStaticFriction = 0.5f;
  tScalar carDynamicFriction = 0.3f;
  tScalar carDensity = 500.0f;
  tScalar carX = 0.0f;
  tScalar carY = 0.0f;
  tScalar carZ = 0.0f;
  GetConfigFile().GetValue("car_x", carX);
  GetConfigFile().GetValue("car_y", carY);
  GetConfigFile().GetValue("car_z", carZ);
  GetConfigFile().GetValue("car_min_x", carMinX);
  GetConfigFile().GetValue("car_min_y", carMinY);
  GetConfigFile().GetValue("car_min_z", carMinZ);
  GetConfigFile().GetValue("car_max_x", carMaxX);
  GetConfigFile().GetValue("car_max_y", carMaxY);
  GetConfigFile().GetValue("car_max_z", carMaxZ);
  GetConfigFile().GetValue("car_elasticity", carElasticity);
  GetConfigFile().GetValue("car_static_friction", carStaticFriction);
  GetConfigFile().GetValue("car_dynamic_friction", carDynamicFriction);
  GetConfigFile().GetValue("car_density", carDensity);


  bool FWDrive = true;
  bool RWDrive = true;
  tScalar maxSteerAngle = 30.0f;
  tScalar steerRate = 4.0f;
  tScalar wheelSideFriction = 4.0f;
  tScalar wheelFwdFriction = 4.0f;
  tScalar wheelTravel = 0.3f;
  tScalar wheelRadius = 0.4f;
  tScalar wheelZOffset = 0.0f;
  tScalar wheelRestingFrac = 0.25f;
  tScalar wheelDampingFrac = 0.6f;
  int     wheelNumRays = 1;
  tScalar driveTorque = 800.0f;
  tScalar gravity = mPhysics.GetGravity().GetLength();
  GetConfigFile().GetValue("FWDrive", FWDrive);
  GetConfigFile().GetValue("RWDrive", RWDrive);
  GetConfigFile().GetValue("max_steer_angle", maxSteerAngle);
  GetConfigFile().GetValue("steer_rate", steerRate);
  GetConfigFile().GetValue("wheel_side_friction", wheelSideFriction);
  GetConfigFile().GetValue("wheel_fwd_friction", wheelFwdFriction);
  GetConfigFile().GetValue("wheel_travel", wheelTravel);
  GetConfigFile().GetValue("wheel_radius", wheelRadius);
  GetConfigFile().GetValue("wheel_z_offset", wheelZOffset);
  GetConfigFile().GetValue("wheel_resting_frac", wheelRestingFrac);
  GetConfigFile().GetValue("wheel_damping_frac", wheelDampingFrac);
  GetConfigFile().GetValue("wheel_num_rays", wheelNumRays);
  GetConfigFile().GetValue("drive_torque", driveTorque);

  tCarObject * carObject = new tCarObject(
    FWDrive, RWDrive, maxSteerAngle, steerRate,
    wheelSideFriction, wheelFwdFriction,
    wheelTravel, wheelRadius, wheelZOffset, wheelRestingFrac, wheelDampingFrac,
    wheelNumRays, driveTorque, gravity);

  carObject->GetCar().GetChassis().SetDims(
    tVector3(carMinX, carMinY, carMinZ),
    tVector3(carMaxX, carMaxY, carMaxZ));
  carObject->SetProperties(carElasticity,
                           carStaticFriction,
                           carDynamicFriction);
  carObject->SetDensity(carDensity);

  carObject->GetBody()->MoveTo(
    tVector3(RangedRandom(-4.0f, 4.0f),
             RangedRandom(-4.0f, 4.0f), 0.0f) +
    tVector3(carX, carY, carZ + 2.0f),
    Matrix33Gamma(RangedRandom(0.0f, 0.0f)));

  // Store
  mObjects.push_back(carObject);
  mCarObjects.push_back(carObject);

  // Register
  mRenderManager.AddObject(carObject);
  carObject->GetCar().EnableCar();

  if (mCarObjects.size() > 1)
    mCarAI.AddControlledCar(carObject);

  return carObject;
}


//======================================================================================================================
// CreateRagdoll
//======================================================================================================================
tRagdoll* tJigTestApp::CreateRagdoll()
{
  tScalar ragdollX = 0.0f;
  tScalar ragdollY = 0.0f;
  tScalar ragdollZ = 0.0f;
  tScalar ragdollDensity = 500.0f;
  bool ragdollStartAsleep = false;
  bool ragdollComplex = false;
  GetConfigFile().GetValue("ragdoll_x", ragdollX);
  GetConfigFile().GetValue("ragdoll_y", ragdollY);
  GetConfigFile().GetValue("ragdoll_z", ragdollZ);
  GetConfigFile().GetValue("ragdoll_density", ragdollDensity);
  GetConfigFile().GetValue("ragdoll_start_asleep", ragdollStartAsleep);
  GetConfigFile().GetValue("ragdoll_complex", ragdollComplex);

  tRagdoll * ragdoll = new tRagdoll(
    &mRenderManager,
    ragdollComplex ? tRagdoll::COMPLEX : tRagdoll::SIMPLE,
    ragdollDensity);

  ragdoll->MoveTorso(
    tVector3(RangedRandom(-4.0f, 4.0f),
             RangedRandom(-4.0f, 4.0f), 0.0f) +
    tVector3(ragdollX, ragdollY, ragdollZ));

  if (ragdollStartAsleep)
    ragdoll->PutToSleep();
  // Store
  mObjects.push_back(ragdoll);
  return ragdoll;
}

//======================================================================================================================
// CreateRagdoll
//======================================================================================================================
tNewton* tJigTestApp::CreateNewton()
{
  tScalar newtonX = 0.0f;
  tScalar newtonY = 0.0f;
  tScalar newtonZ = 0.0f;
  tScalar newtonRadius = 1.0f;
  int balls = 5;
  GetConfigFile().GetValue("newton_x", newtonX);
  GetConfigFile().GetValue("newton_y", newtonY);
  GetConfigFile().GetValue("newton_z", newtonZ);
  GetConfigFile().GetValue("newton_radius", newtonRadius);
  GetConfigFile().GetValue("newton_balls", balls);

  static tScalar yOffset = 0.0f;

  tNewton * newton = new tNewton(
    &mRenderManager,
    balls,
    tVector3(newtonX, newtonY + yOffset, newtonZ),
    newtonRadius);

  yOffset += newtonRadius * 4.0f;
  mObjects.push_back(newton);
  return newton;
}

//======================================================================================================================
// SelectObject
//======================================================================================================================
void tJigTestApp::SelectObject(int index)
{
  if (index < 0 || index >= (int) mObjects.size())
    index = -1;

  tCarObject * oldCar = GetSelectedCar();
  mSelectedIndex = index;
  tCarObject * newCar = GetSelectedCar();
  if (oldCar == newCar)
    return;

  // The selected car is driven with the arrow keys. Other cars (apart from the first one) are
  // driven by the AI.
  if (oldCar)
  {
    oldCar->GetCar().SetAccelerate(0.0f);
    oldCar->GetCar().SetSteer(0.0f);
    if (oldCar != mCarObjects[0])
      mCarAI.AddControlledCar(oldCar);
  }
  if (newCar)
    mCarAI.RemoveControlledCar(newCar);
}

//======================================================================================================================
// SelectNextObject
//======================================================================================================================
void tJigTestApp::SelectNextObject(int step)
{
  int n = (int) mObjects.size();
  if (n == 0)
    return;
  int index = mSelectedIndex;
  if (index < 0)
    index = (step > 0) ? n - 1 : 0;
  for (int count = 0 ; count < n ; ++count)
  {
    index = ((index + step) % n + n) % n;
    if (mObjects[index]->GetBody())
    {
      SelectObject(index);
      return;
    }
  }
}

//======================================================================================================================
// GetSelectedObject
//======================================================================================================================
tObject * tJigTestApp::GetSelectedObject()
{
  if (mSelectedIndex < 0 || mSelectedIndex >= (int) mObjects.size())
    return 0;
  return mObjects[mSelectedIndex];
}

//======================================================================================================================
// GetSelectedCar
//======================================================================================================================
tCarObject * tJigTestApp::GetSelectedCar()
{
  tObject * obj = GetSelectedObject();
  for (tCarObject * car : mCarObjects)
  {
    if (car == obj)
      return car;
  }
  return 0;
}

//======================================================================================================================
// GetViewportAt
//======================================================================================================================
const tViewport * tJigTestApp::GetViewportAt(const tVector2 & mousePt)
{
  for (const tViewport * viewport : mViewports)
  {
    tVector2 pos, size;
    viewport->GetPositionAndSize(pos, size);
    if (mousePt.x > pos.x &&
        mousePt.x < pos.x + size.x &&
        mousePt.y > pos.y &&
        mousePt.y < pos.y + size.y)
    {
      return viewport;
    }
  }
  return mViewports.empty() ? 0 : mViewports[0];
}

//======================================================================================================================
// PickPoint
//======================================================================================================================
bool tJigTestApp::PickPoint(const tVector2 & mousePt, tVector3 & hitPos, tBody *& body)
{
  body = 0;
  const tViewport * viewport = GetViewportAt(mousePt);
  tCollisionSystem * coll = GetPhysicsSystem()->GetCollisionSystem();
  if (!viewport || !coll)
    return false;

  const tCamera * camera = viewport->GetCamera();
  tVector3 worldPos = camera->Unproject(mousePt, viewport);
  const tVector3 & camPos = camera->GetPosition();
  tVector3 dir = (worldPos - camPos).GetNormalisedSafe();
  tSegment seg(camPos, 500.0f * dir);

  tScalar frac = 0;
  tCollisionSkin * skin = 0;
  tVector3 normal;
  if (!coll->SegmentIntersect(frac, skin, hitPos, normal, seg, 0) || !skin)
    return false;
  body = skin->GetOwner();
  return true;
}

//======================================================================================================================
// RotateAbout
//======================================================================================================================
// Rotates v about a unit axis by an angle in degrees
static tVector3 RotateAbout(const tVector3 & v, const tVector3 & axis, tScalar degrees)
{
  tScalar angle = Deg2Rad(degrees);
  tScalar c = Cos(angle);
  tScalar s = Sin(angle);
  return c * v + s * Cross(axis, v) + ((1.0f - c) * Dot(axis, v)) * axis;
}

//======================================================================================================================
// GetCameraDirection
//======================================================================================================================
tVector3 tJigTestApp::GetCameraDirection() const
{
  tScalar yaw = Deg2Rad(mCameraYaw);
  tScalar pitch = Deg2Rad(mCameraPitch);
  return tVector3(Cos(pitch) * Cos(yaw), Cos(pitch) * Sin(yaw), Sin(pitch));
}

//======================================================================================================================
// SetCameraDirection
//======================================================================================================================
void tJigTestApp::SetCameraDirection(const tVector3 & dir)
{
  if (dir.GetLengthSq() < SCALAR_TINY)
    return;
  tVector3 d = dir.GetNormalised();
  mCameraYaw = Rad2Deg(ATan2(d.y, d.x));
  mCameraPitch = Rad2Deg(ASin(d.z));
  Limit(mCameraPitch, SCALAR(-89.0f), SCALAR(89.0f));
}

//======================================================================================================================
// GetOrbitPivot
//======================================================================================================================
tVector3 tJigTestApp::GetOrbitPivot()
{
  if (tObject * obj = GetSelectedObject())
    return obj->GetRenderPosition();
  return mOrbitPoint;
}

//======================================================================================================================
// RotateCamera
//======================================================================================================================
void tJigTestApp::RotateCamera(tScalar deltaYaw, tScalar deltaPitch, bool orbit)
{
  tScalar newPitch = mCameraPitch + deltaPitch;
  Limit(newPitch, SCALAR(-89.0f), SCALAR(89.0f));
  deltaPitch = newPitch - mCameraPitch;

  if (orbit && !mCameras.empty())
  {
    // Move the camera around the pivot by the same rotation that's applied to its direction, so the
    // pivot stays in the same place on screen
    tCamera * camera = mCameras[0];
    tVector3 pivot = GetOrbitPivot();
    tVector3 offset = RotateAbout(camera->GetPosition() - pivot, tVector3::Up(), deltaYaw);
    tScalar newYaw = Deg2Rad(mCameraYaw + deltaYaw);
    tVector3 left(-Sin(newYaw), Cos(newYaw), 0.0f);
    // A positive rotation about the left axis tips the view down
    offset = RotateAbout(offset, left, -deltaPitch);
    camera->SetPosition(pivot + offset);
  }

  mCameraYaw += deltaYaw;
  if (mCameraYaw > 180.0f)
    mCameraYaw -= 360.0f;
  else if (mCameraYaw < -180.0f)
    mCameraYaw += 360.0f;
  mCameraPitch = newPitch;
}

//======================================================================================================================
// UpdateFreeCamera
//======================================================================================================================
void tJigTestApp::UpdateFreeCamera(tScalar dt)
{
  if (mCameras.empty())
    return;
  tCamera * camera = mCameras[0];

  // The arrow keys turn the camera (unless a car is selected)
  tScalar lookAngle = 90.0f * dt;
  tScalar yaw = 0.0f, pitch = 0.0f;
  if (mCameraLookDir & LOOK_LEFT)
    yaw += lookAngle;
  if (mCameraLookDir & LOOK_RIGHT)
    yaw -= lookAngle;
  if (mCameraLookDir & LOOK_UP)
    pitch += lookAngle;
  if (mCameraLookDir & LOOK_DOWN)
    pitch -= lookAngle;
  if (yaw != 0.0f || pitch != 0.0f)
    RotateCamera(yaw, pitch, false);

  // W/S move along the view direction, A/D to the side, and Q/E down and up
  tVector3 dir = GetCameraDirection();
  tVector3 left = Cross(tVector3::Up(), dir).GetNormalisedSafe();
  tVector3 move(0.0f);
  if (mCameraMoveDir & MOVE_FWD)
    move += dir;
  if (mCameraMoveDir & MOVE_BACK)
    move -= dir;
  if (mCameraMoveDir & MOVE_LEFT)
    move += left;
  if (mCameraMoveDir & MOVE_RIGHT)
    move -= left;
  if (mCameraMoveDir & MOVE_UP)
    move += tVector3::Up();
  if (mCameraMoveDir & MOVE_DOWN)
    move -= tVector3::Up();

  tVector3 pos = camera->GetPosition();
  if (move.GetLengthSq() > SCALAR_TINY)
  {
    tScalar speed = tAppConfig::mCamSpeed;
    if (SDL_GetModState() & SDL_KMOD_SHIFT)
      speed *= 4.0f;
    pos += (speed * dt) * move.GetNormalised();
  }

  camera->SetPosition(pos);
  camera->SetTarget(pos + dir);
  camera->SetUp(tVector3::Up());
}

//======================================================================================================================
// StartMouseLook
//======================================================================================================================
void tJigTestApp::StartMouseLook(int mode)
{
  mMouseMode = (tMouseMode) mode;
  // Hide the pointer and report relative movement, so the drag isn't limited by the window edge
  SDL_SetWindowRelativeMouseMode(GetMainWindow(), true);
}

//======================================================================================================================
// StopMouseLook
//======================================================================================================================
void tJigTestApp::StopMouseLook()
{
  mMouseMode = MOUSE_NONE;
  SDL_SetWindowRelativeMouseMode(GetMainWindow(), false);
}

//======================================================================================================================
// UpdateDrag
//======================================================================================================================
void tJigTestApp::UpdateDrag()
{
  tBody * body = mObjectPositionController.GetBody();
  if (!mPicking || !body || !mObjectPositionController.GetConstraintEnabled())
    return;
  body->SetActive();

  const tViewport * viewport = GetViewportAt(mMousePos);
  if (!viewport)
    return;
  tVector3 worldPos = viewport->GetCamera()->Unproject(mMousePos, viewport);
  const tVector3 & camPos = viewport->GetCamera()->GetPosition();
  tVector3 dir = (worldPos - camPos).GetNormalisedSafe();
  mObjectPositionController.SetWorldPosition(camPos + mPickDist * dir);
}

//======================================================================================================================
// StopDrag
//======================================================================================================================
void tJigTestApp::StopDrag()
{
  if (mObjectPositionController.GetConstraintEnabled() && mObjectPositionController.GetBody())
    mObjectPositionController.GetBody()->SetActive();
  mObjectPositionController.DisableConstraint();
  mObjectPositionDamperController.DisableConstraint();
  mPicking = false;
}

//======================================================================================================================
// RenderHelp
//======================================================================================================================
void tJigTestApp::RenderHelp()
{
  static const char * helpLines[] = {
    "#Camera",
    "W  A  S  D\tmove (hold Shift to go faster)",
    "Q  E\tmove down / up",
    "Right drag\tlook around",
    "Arrow keys\tlook around (or drive the selected car)",
    "Alt + left drag\torbit around the selected object",
    "Mouse wheel\tmove forwards / backwards",
    "#Objects",
    "Left click\tselect",
    "Left drag\tpull it around (mouse wheel: nearer / further)",
    "Middle click\tthrow the object you're dragging",
    "Space\tfire the selected object from the camera",
    "=  -\tselect the next / previous object",
    "X  Z  N\tdrop a box, sphere or capsule",
    "/  \\\tdrop a compound object or a ragdoll",
    "C\tdrop a car (select it to drive it)",
    "Ctrl + any of these\tfire it from the camera",
    "M\tfirst-person character (W A S D, mouse, Space)",
    "T  Y\treset / remove all the objects",
    "#Simulation",
    "P  O\tpause / single step",
    "F  G\tfreezing / shock step on or off",
    "K  B  V\tshow contacts / bounding boxes / sleeping",
    "Page Up / Down\ttexture detail",
    "R\trendering on or off",
    "F1  F2\tscreenshot / record frames",
    "H\thide this help",
    "Esc\tquit",
  };
  static const int numHelpLines = (int) (sizeof(helpLines) / sizeof(helpLines[0]));

  // Use the same size for the reminder as the full help, so it doesn't jump around
  float scale = GetTextPanelScale(numHelpLines);
  std::vector<std::string> lines;
  if (mShowHelp)
  {
    lines.assign(helpLines, helpLines + numHelpLines);
  }
  else
  {
    lines.push_back("H\thelp");
    if (GetSelectedCar())
      lines.push_back("Arrow keys\tdrive the selected car");
  }
  DrawTextPanel(lines, 10, 10, scale);
}
