//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
#ifndef DEBUGCONFIG_HPP
#define DEBUGCONFIG_HPP

// Global config - for debugging
class tDebugConfig
{
public:
  static bool mIndicateFrozenObjects;
  static bool mRenderCollisionBoundingBoxes;
  static int  mRenderCollisionContacts;
};

#endif
