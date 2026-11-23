//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef JIGCOLLISIONSYSTEMBRUTE_HPP
#define JIGCOLLISIONSYSTEMBRUTE_HPP

#include "../collision/include/collisionsystem.hpp"

#include <vector>

// Implements a "brute-force" collision system with no culling other than bounding volume checks.

namespace JigLib
{
class tCollisionSystemBrute : public tCollisionSystem
{
public:
  tCollisionSystemBrute();
  ~tCollisionSystemBrute();

  // Inherited
  void AddCollisionSkin(class tCollisionSkin * skin);

  // Inherited
  bool RemoveCollisionSkin(class tCollisionSkin * skin);

  // Inherited
  void DetectCollisions(
    class tBody & body,
    tCollisionFunctor & collisionFunctor,
    const tCollisionSkinPredicate2 * collisionPredicate,
    tScalar collTolerance);

  // Inherited
  void DetectAllCollisions(
    const std::vector<class tBody *> & bodies,
    tCollisionFunctor & collisionFunctor,
    const tCollisionSkinPredicate2 * collisionPredicate,
    tScalar collTolerance);

  bool SegmentIntersect(
    tScalar & frac,
    tCollisionSkin *& skin,
    tVector3 & pos,
    tVector3 & normal,
    const class tSegment & seg,
    const tCollisionSkinPredicate1 * collisionPredicate);

private:
  typedef std::vector<tCollisionSkin *> tSkins;
  tSkins mSkins;

  bool mDetecting;
};
}

#endif
