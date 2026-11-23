//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef JIGTESTAPP_HPP
#define JIGTESTAPP_HPP

#include "sdlapplicationbase.hpp"
#include "rendermanager.hpp"
#include "carai.hpp"

#include "jiglib.hpp"

#include <vector>
#include <string>

class tJigTestApp : public tSDLApplicationBase
{
public:
  tJigTestApp(int & argc, char * argv[]);
  ~tJigTestApp();

  // Inherited from tSDLApplicationBase
  class JigLib::tPhysicsSystem * GetPhysicsSystem() {return &mPhysics;}
  bool Initialise(const std::string app_name);
  void StartMainLoop();

  // event handlers
  void HandleKeyDown(const SDL_KeyboardEvent & event );
  void HandleKeyUp(const SDL_KeyboardEvent & event );
  void HandleFPSKeyDown(const SDL_KeyboardEvent & event );
  void HandleFPSKeyUp(const SDL_KeyboardEvent & event );
  void HandleFPSMouseMotion(const SDL_MouseMotionEvent & event);
  void HandleMouseMotion(const SDL_MouseMotionEvent & event);
  void HandleMouseButtonDown(const SDL_MouseButtonEvent & event);
  void HandleMouseButtonUp(const SDL_MouseButtonEvent & event);
  void HandleMouseWheel(const SDL_MouseWheelEvent & event);
  void ProcessMainEvent();

  void AddObject(class tObject * object);

private:
  void LoadAppConfig();

  // height is the number of levels. pos is the position of the centre of the bottom of the stack
  void BuildJengaStack(int height,
                       const JigLib::tVector3 & boxDims,
                       const JigLib::tVector3 & pos);

  void BuildWall(int height,
                 int width,
                 const JigLib::tVector3 & boxDims,
                 const JigLib::tVector3 & pos,
                 bool pyramid);

  void CreateBumpyHeightmap();

  void CreateFileHeightmap(const std::string & fileName);

  void SetupExperiment();

  void SetupUnequalExperiment();

  // Fires an object (index into mObjects) from the camera
  void FireObject(int index);

  // Fires the last created object, but doesn't change the selection
  void FireLastCreatedObject();

  void RemoveAllObjects(bool onlyPhysical);

  void RestartExperiment();

  void SetSleepingParams();

  class tBoxObject * CreateBox();

  class tBoxSphereObject * CreateCompound();

  class tSphereObject * CreateSphere();

  class tCapsuleObject * CreateCapsule();

  class tCarObject * CreateCar();

  class tCharacterObject * CreateCharacter();

  // Deletes the selected object
  void DeleteSelectedObject();

  // Removes the first-person character(s) and goes back to the free camera
  void RemoveCharacters();

  class tRagdoll * CreateRagdoll();

  class tNewton * CreateNewton();

  enum tObjectControl {CONTROL_UP, CONTROL_DOWN, CONTROL_LEFT, CONTROL_RIGHT};
  void ControlCar(class tCarObject * car, tObjectControl control, bool down);

  // Selection. The index is into mObjects, or -1 for nothing selected.
  void SelectObject(int index);
  // Steps through the objects that have bodies, forwards (step = 1) or backwards (step = -1)
  void SelectNextObject(int step);
  class tObject * GetSelectedObject();
  class tCarObject * GetSelectedCar();

  // Finds what's under the mouse. Returns false if nothing is hit. body is 0 if what was hit has
  // no body (e.g. the ground).
  bool PickPoint(const JigLib::tVector2 & mousePt, JigLib::tVector3 & hitPos, JigLib::tBody *& body);
  const class tViewport * GetViewportAt(const JigLib::tVector2 & mousePt);

  // The free camera
  void UpdateFreeCamera(JigLib::tScalar dt);
  // Turns the camera by these angles (degrees). If orbit is set then the camera also moves around
  // the orbit pivot.
  void RotateCamera(JigLib::tScalar deltaYaw, JigLib::tScalar deltaPitch, bool orbit);
  void SetCameraDirection(const JigLib::tVector3 & dir);
  JigLib::tVector3 GetCameraDirection() const;
  JigLib::tVector3 GetOrbitPivot();
  void StartMouseLook(int mode);
  void StopMouseLook();

  // Moves the dragged object to follow the mouse
  void UpdateDrag();
  void StopDrag();

  void RenderHelp();

  enum tMoveDir {MOVE_NONE  = 1 << 0,
                 MOVE_FWD   = 1 << 1,
                 MOVE_BACK  = 1 << 2,
                 MOVE_LEFT  = 1 << 3,
                 MOVE_RIGHT = 1 << 4,
                 MOVE_UP    = 1 << 5,
                 MOVE_DOWN  = 1 << 6};

  // Our physics system
  JigLib::tPhysicsSystem mPhysics;

  // Create the collision system on the heap since we might want to choose the type at run-time
  // (from config)
  JigLib::tCollisionSystem * mCollisionSystem;

  // The thing we use to manager all renderable objects
  tRenderManager mRenderManager;

  // Could have a number of viewports...
  std::vector<class tViewport *> mViewports;

  // Could have a number of cameras, for example one per viewport
  std::vector<class tCamera *> mCameras;

  // Used to move objects by constraining their velocity
  JigLib::tConstraintVelocity mObjectController;

  // Used to move objects by constraining their position
  JigLib::tConstraintWorldPoint mObjectPositionController;
  // Damps velocity on the position-controlled object
  JigLib::tConstraintVelocity mObjectPositionDamperController;

  // All our things
  typedef std::vector<class tObject *> tObjects;
  tObjects mObjects;

  // Additional list for cars so that the user can control them
  typedef std::vector<class tCarObject *> tCarObjects;
  tCarObjects mCarObjects;

  // Additional list for characters so that the user can control them
  typedef std::vector<class tCharacterObject *> tCharacterObjects;
  tCharacterObjects mCharacterObjects;

  // Index into mObjects of the selected object, or -1 if nothing is selected
  int mSelectedIndex;

  // For camera movement
  unsigned mCameraMoveDir;
  // Actual camera vel
  JigLib::tVector3 mCameraMoveVel;

  // Free camera direction in degrees. Yaw is about the up axis, measured from the x axis.
  JigLib::tScalar mCameraYaw, mCameraPitch;

  // Arrow keys held down for looking around
  enum tLookDir {LOOK_LEFT = 1 << 0, LOOK_RIGHT = 1 << 1, LOOK_UP = 1 << 2, LOOK_DOWN = 1 << 3};
  unsigned mCameraLookDir;

  // What dragging the mouse does
  enum tMouseMode {MOUSE_NONE, MOUSE_LOOK, MOUSE_ORBIT};
  tMouseMode mMouseMode;
  JigLib::tVector2 mMousePos;

  // The camera orbits around the selected object, or this point (the last place clicked)
  JigLib::tVector3 mOrbitPoint;

  // Show the help overlay
  bool mShowHelp;

  // The average duration of each physics step
  JigLib::tScalar mPhysicsDuration;

  // Pause physics
  bool mPaused;

  // Allow single-stepping
  bool mPauseAfterNextPhysicsStep;

  // AI controlled cars
  tCarAI mCarAI;

  // Maybe have a skybox
  class tSkybox * mSkybox;

  // true if we're picking/moving an object with the mouse
  bool mPicking;

  // When picking this is the original distance from the camera to the object
  JigLib::tScalar mPickDist;

  // Record a movie?
  bool mDoMovie;

  // If non-zero then quit after this many physics updates -
  // e.g. when profiling
  int mQuitIterations;

  // Counts the number of physics updates we've done
  int mPhysicsCounter;
};

#endif
