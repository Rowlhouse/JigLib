//==============================================================
// Copyright (C) 2004 Danny Chapman 
//               danny@rowlhouse.freeserve.co.uk
//--------------------------------------------------------------
//               
/// @file timer.hpp 
//                     
//==============================================================
#ifndef JIGTIMER_HPP
#define JIGTIMER_HPP

#include "../utils/include/time.hpp"

#include <chrono>

namespace JigLib
{
  typedef std::chrono::steady_clock::time_point tHighResTimeVal;

  // convert time delta into useable values
  tTime GetTimeDelta(tHighResTimeVal t0, tHighResTimeVal t1);
  
  /// available indicates if a high-resolution time was available. If
  /// not then don't believe what this returns.
  tHighResTimeVal GetHighResTime(bool & available);
  
}

#endif
