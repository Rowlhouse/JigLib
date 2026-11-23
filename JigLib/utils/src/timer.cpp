//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "timer.hpp"

using namespace JigLib;

tTime JigLib::GetTimeDelta(tHighResTimeVal t0, tHighResTimeVal t1)
{
  return (tTime) std::chrono::duration<double>(t1 - t0).count();
}

tHighResTimeVal JigLib::GetHighResTime(bool & available)
{
  available = true;
  return std::chrono::steady_clock::now();
}
