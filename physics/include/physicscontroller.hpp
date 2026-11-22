//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
#ifndef JIGPHYSICSCONTROLLER_HPP
#define JIGPHYSICSCONTROLLER_HPP
#include "../maths/include/precision.hpp"

namespace JigLib
{
// This can get updated at the same time as tBody::AddExternalForces so that forces
// can be added independent of individual bodies - e.g. joints between pairs of bodies.
class tPhysicsController
{
public:
  tPhysicsController();
  virtual ~tPhysicsController();

  // Register with the physics system
  void EnableController();

  // Deregister from the physics system
  void DisableController();

  // Are we registered with the physics system?
  bool GetControllerEnabled() const {return mControllerEnabled;}

private:
  friend class tPhysicsSystem;
  // Implement this to apply whatever forces are needed to the objects this controls
  virtual void UpdateController(tScalar dt) = 0;

private:
  bool mControllerEnabled;
};
}

#endif
