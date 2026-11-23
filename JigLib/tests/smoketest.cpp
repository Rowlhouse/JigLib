//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
// A quick check that JigLib builds, links and simulates sensibly, without any graphics. It drops
// some objects onto a ground plane and checks where they come to rest. It also serves as a minimal
// example of setting up and stepping a JigLib simulation.
//
// Returns 0 if all the checks pass.
//======================================================================================================================
#include "jiglib.hpp"

#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

using namespace JigLib;

static int sFailures = 0;

static void Check(bool ok, const char * what)
{
  printf("  %s: %s\n", ok ? "pass" : "FAIL", what);
  if (!ok)
    ++sFailures;
}

static bool IsFinite(const tVector3 & v)
{
  return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

//======================================================================================================================
// A rigid body with a collision skin made from one primitive - the same way the JigTest objects
// are set up.
//======================================================================================================================
struct tSimpleObject
{
  tSimpleObject(const tPrimitive & primitive, tScalar mass, const tVector3 & pos)
  {
    mSkin.AddPrimitive(primitive, tMaterialTable::UNSET, tMaterialProperties(0.2f, 0.8f, 0.6f));
    mSkin.SetOwner(&mBody);
    mBody.SetCollisionSkin(&mSkin);

    // Calculate the inertia, and move the body so that its origin is at the centre of mass
    mBody.SetMass(mass);
    tPrimitive::tPrimitiveProperties properties(tPrimitive::tPrimitiveProperties::SOLID,
                                                tPrimitive::tPrimitiveProperties::MASS, mass);
    tScalar junk;
    tVector3 com;
    tMatrix33 inertia, inertiaCoM;
    mSkin.GetMassProperties(properties, junk, com, inertia, inertiaCoM);
    mSkin.ApplyLocalTransform(tTransform3(-com, tMatrix33::Identity()));
    mBody.SetBodyInertia(inertiaCoM);

    mBody.MoveTo(pos, tMatrix33::Identity());
    // This registers the body (and its skin) with the current physics system
    mBody.EnableBody();
  }

  tBody mBody;
  tCollisionSkin mSkin;
};

//======================================================================================================================
// A physics system with a ground plane at z = 0
//======================================================================================================================
struct tWorld
{
  tWorld()
  {
    mPhysics.SetCollisionSystem(&mCollisionSystem);
    mPhysics.SetGravity(tVector3(0.0f, 0.0f, -10.0f));
    mPhysics.SetSolverType(tPhysicsSystem::SOLVER_ACCUMULATED);
    mPhysics.SetNumCollisionIterations(2);
    mPhysics.SetNumContactIterations(7);
    mPhysics.SetNumPenetrationRelaxationTimesteps(2);
    mPhysics.SetAllowedPenetration(0.01f);
    mPhysics.SetCollToll(0.05f);
    mPhysics.EnableFreezing(true);

    // The ground doesn't have a body - its skin is just added to the collision system
    mGround.AddPrimitive(tPlane(tVector3::Up(), 0.0f), tMaterialTable::UNSET,
                         tMaterialProperties(0.2f, 0.8f, 0.6f));
    mCollisionSystem.AddCollisionSkin(&mGround);
  }

  void Run(tScalar seconds)
  {
    const tScalar dt = 1.0f / 60.0f;
    int steps = (int) (seconds / dt);
    for (int i = 0 ; i < steps ; ++i)
      mPhysics.Integrate(dt);
  }

  tSimpleObject & Add(const tPrimitive & primitive, tScalar mass, const tVector3 & pos)
  {
    mObjects.push_back(std::make_unique<tSimpleObject>(primitive, mass, pos));
    return *mObjects.back();
  }

  // Declaration order matters for destruction: the physics system goes first (which unregisters
  // the bodies), then the objects and ground, then the collision system.
  tCollisionSystemBrute mCollisionSystem;
  tCollisionSkin mGround;
  std::vector<std::unique_ptr<tSimpleObject>> mObjects;
  tPhysicsSystem mPhysics;
};

//======================================================================================================================
// Tests
//======================================================================================================================
static void TestBoxComesToRest()
{
  printf("Box dropped onto the ground\n");
  tWorld world;
  tVector3 sides(1.0f, 1.0f, 1.0f);
  tBody & box = world.Add(tBox(-0.5f * sides, tMatrix33::Identity(), sides), 1.0f,
                          tVector3(0.0f, 0.0f, 2.0f)).mBody;
  world.Run(4.0f);

  const tVector3 & pos = box.GetPosition();
  printf("  position (%.3f, %.3f, %.3f)\n", pos.x, pos.y, pos.z);
  Check(IsFinite(pos), "position is finite");
  Check(std::fabs(pos.z - 0.5f) < 0.05f, "resting on the ground");
  Check(box.GetVelocity().GetLength() < 0.05f, "stopped moving");
  Check(std::fabs(pos.x) < 0.05f && std::fabs(pos.y) < 0.05f, "didn't drift sideways");
}

static void TestSphereComesToRest()
{
  printf("Sphere dropped onto the ground\n");
  tWorld world;
  tBody & sphere = world.Add(tSphere(tVector3::Zero(), 0.5f), 1.0f, tVector3(0.0f, 0.0f, 3.0f)).mBody;
  world.Run(4.0f);

  const tVector3 & pos = sphere.GetPosition();
  printf("  position (%.3f, %.3f, %.3f)\n", pos.x, pos.y, pos.z);
  Check(IsFinite(pos), "position is finite");
  Check(std::fabs(pos.z - 0.5f) < 0.05f, "resting on the ground");
  Check(sphere.GetVelocity().GetLength() < 0.05f, "stopped moving");
}

static void TestBoxStackStands()
{
  printf("Stack of five boxes\n");
  tWorld world;
  tVector3 sides(1.0f, 1.0f, 1.0f);
  std::vector<tBody *> boxes;
  for (int i = 0 ; i < 5 ; ++i)
  {
    tVector3 pos(0.0f, 0.0f, 0.5f + i * 1.01f);
    boxes.push_back(&world.Add(tBox(-0.5f * sides, tMatrix33::Identity(), sides), 1.0f, pos).mBody);
  }
  world.Run(6.0f);

  bool allFinite = true;
  for (tBody * box : boxes)
    allFinite = allFinite && IsFinite(box->GetPosition());
  const tVector3 & top = boxes.back()->GetPosition();
  printf("  top box position (%.3f, %.3f, %.3f)\n", top.x, top.y, top.z);
  Check(allFinite, "positions are finite");
  Check(std::fabs(top.z - 4.5f) < 0.1f, "top box is still at the top");
  Check(std::fabs(top.x) < 0.1f && std::fabs(top.y) < 0.1f, "stack is still upright");
}

//======================================================================================================================
// main
//======================================================================================================================
int main()
{
  printf("JigLib version %d smoke test\n", GetVersion());

  TestBoxComesToRest();
  TestSphereComesToRest();
  TestBoxStackStands();

  if (sFailures == 0)
    printf("All checks passed\n");
  else
    printf("%d check(s) failed\n", sFailures);
  return sFailures == 0 ? 0 : 1;
}
