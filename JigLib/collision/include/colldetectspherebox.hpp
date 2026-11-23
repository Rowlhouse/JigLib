//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef JIGCOLLDETECTSPHEREBOX_HPP
#define JIGCOLLDETECTSPHEREBOX_HPP

#include "../collision/include/collisionsystem.hpp"
namespace JigLib
{
class tCollDetectSphereBox : public tCollDetectFunctor
{
public:
  tCollDetectSphereBox();

  void CollDetect(const tCollDetectInfo &info,
                  tScalar collTolerance,
                  tCollisionFunctor & collisionFunctor) const;
};
}

#endif
