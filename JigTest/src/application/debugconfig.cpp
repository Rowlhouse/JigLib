//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "debugconfig.hpp"

bool tDebugConfig::mIndicateFrozenObjects;
bool tDebugConfig::mRenderCollisionBoundingBoxes;
int tDebugConfig::mRenderCollisionContacts;

//======================================================================================================================
// ResetToDefaults
//======================================================================================================================
void tDebugConfig::ResetToDefaults()
{
  mIndicateFrozenObjects = false;
  mRenderCollisionBoundingBoxes = false;
  mRenderCollisionContacts = 0;
}
