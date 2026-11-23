//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef JIGCOLLDETECTSPHEREPLANE_HPP
#define JIGCOLLDETECTSPHEREPLANE_HPP

#include "../collision/include/collisionsystem.hpp"
namespace JigLib
{
class tCollDetectSpherePlane : public tCollDetectFunctor
{
public:
  tCollDetectSpherePlane();

  void CollDetect(const tCollDetectInfo &info,
                  tScalar collTolerance,
                  tCollisionFunctor & collisionFunctor) const;
};
}

#endif
