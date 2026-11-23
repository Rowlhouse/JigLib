//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef JIGLIB_CONFIG_HPP
#define JIGLIB_CONFIG_HPP

//#define USING_DOUBLE

// disable MSVC specific warnings
#ifdef _MSC_VER
// long names in debug info
#pragma warning (disable : 4786)
// Massive boost if we disable exception handling but MS generates warnings
#pragma warning(disable: 4530)
#endif

#endif
