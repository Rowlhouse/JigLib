//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
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

  // Sets everything to its default value (before reading a config file)
  static void ResetToDefaults();
};

#endif
