//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "appconfig.hpp"

using namespace JigLib;

int tAppConfig::mCollisionSystemType;
tScalar tAppConfig::mCollisionGridSize;
bool tAppConfig::mEnableFreezing;
int tAppConfig::mNumCollisionIterations;
int tAppConfig::mNumContactIterations;
int tAppConfig::mNumPenetrationRelaxationTimesteps;
tScalar tAppConfig::mAllowedPenetration;
bool tAppConfig::mDoShockStep;
tScalar tAppConfig::mPhysicsFrequency;
std::string tAppConfig::mSolverType;
int tAppConfig::mMaxPhysicsStepsPerFrame;
bool tAppConfig::mNullPhysicsUpdate;
tVector3 tAppConfig::mGravity;
tScalar tAppConfig::mProjectileSpeed;
tScalar tAppConfig::mPhysicsCollToll;
tScalar tAppConfig::mPhysicsVelThreshold;
tScalar tAppConfig::mPhysicsAngVelThreshold;
tScalar tAppConfig::mPhysicsPosThreshold;
tScalar tAppConfig::mPhysicsOrientThreshold;
tScalar tAppConfig::mPhysicsDeactivationTime;
bool tAppConfig::mRenderEnable;
int tAppConfig::mTextureLevel;
bool tAppConfig::mRenderInterpolate;
tScalar tAppConfig::mCamSpeed;
tScalar tAppConfig::mObjectControllerSpeed;
std::string tAppConfig::mSkybox;
int tAppConfig::mMovieInterval;

//======================================================================================================================
// ResetToDefaults
//======================================================================================================================
void tAppConfig::ResetToDefaults()
{
  mCollisionSystemType = 0;
  mCollisionGridSize = 10.0f;
  mEnableFreezing = true;
  mNumCollisionIterations = 4;
  mNumContactIterations = 4;
  mNumPenetrationRelaxationTimesteps = 7;
  mAllowedPenetration = 0.0001f;
  mDoShockStep = false;
  mPhysicsFrequency = 100.0f;
  mSolverType = std::string("accumulated");
  mMaxPhysicsStepsPerFrame = 2;
  mNullPhysicsUpdate = false;
  mGravity = tVector3(0.0f, 0.0f, 0.0f);
  mProjectileSpeed = 40.0f;
  mPhysicsCollToll = 0.005f;
  mPhysicsVelThreshold = 0.4f;
  mPhysicsAngVelThreshold = 10.0f;
  mPhysicsPosThreshold = 0.1f;
  mPhysicsOrientThreshold = 0.2f;
  mPhysicsDeactivationTime = 1.0f;
  mRenderEnable = true;
  mTextureLevel = 0;
  mRenderInterpolate = true;
  mCamSpeed = 10.0f;
  mObjectControllerSpeed = 10.0f;
  mSkybox = std::string("none");
  mMovieInterval = 2;
}
