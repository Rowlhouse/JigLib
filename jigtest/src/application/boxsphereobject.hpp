//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
#ifndef BOXSPHEREOBJECT_HPP
#define BOXSPHEREOBJECT_HPP

#include "object.hpp"
#include "graphics.hpp"
#include "jiglib.hpp"

// This is a game-object - i.e. a box/sphere compound that has a body, has collision, and can be
// rendered.
class tBoxSphereObject : public tObject
{
public:
  tBoxSphereObject(JigLib::tVector3 sides, bool randomColour);
  ~tBoxSphereObject();
  void SetProperties(JigLib::tScalar elasticity,
                     JigLib::tScalar staticFriction,
                     JigLib::tScalar dynamicFriction);
  // Inherited from tObject - uses a solid mass distribution
  void SetDensity(JigLib::tScalar density) override
    {SetDensity(density, JigLib::tPrimitive::tPrimitiveProperties::SOLID);}
  void SetDensity(JigLib::tScalar density,
                  JigLib::tPrimitive::tPrimitiveProperties::tMassDistribution massDistribution);
  void SetMass(JigLib::tScalar mass,
               JigLib::tPrimitive::tPrimitiveProperties::tMassDistribution massDistribution =
                 JigLib::tPrimitive::tPrimitiveProperties::SOLID);

  JigLib::tBody * GetBody() {return &mBody;}

  // Inherited from tObject - interpolated between the physics old and current positions.
  void SetRenderPosition(JigLib::tScalar renderFraction);

  // Inherited from tRenderObject
  void Render(tRenderType renderType);
  const JigLib::tSphere & GetRenderBoundingSphere() const;
  const JigLib::tVector3 & GetRenderPosition() const;
  const JigLib::tMatrix33 & GetRenderOrientation() const;

private:
  JigLib::tBody mBody;
  JigLib::tCollisionSkin mCollisionSkin;

  JigLib::tVector3 mRenderPosition;
  JigLib::tMatrix33 mRenderOrientation;

  bool mRandomColour;
  class tRGBAFileTexture* mTexture;

  GLuint mDisplayListNum;
};

#endif
