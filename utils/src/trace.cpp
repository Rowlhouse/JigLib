// JigLib - Copyright (C) 2004 Danny Chapman
//
// \file trace.cpp

#include "trace.hpp"
#include <stdio.h>
#include <filesystem>
#include <string>

namespace JigLib
{
  
  
/// Overall trace enabled
  bool traceEnabled = false;
  
/// The overall trace level - only trace with a level equal to or less
/// than this comes out.
  int traceLevel = 0;
  
/// The strings for which trace is enabled. Normally these will be
/// file names, though they don't have to be. 
  std::vector<std::string> traceStrings;
  
/// If this flag is set, all trace strings are enabled
  bool traceAllStrings = false;
  
  void TracePrintf(const char *fmt, ...)
  {
    va_list ap;
    
    // prepare log file
    static bool init = false;
    static FILE * logFile = 0;
    
    if (init == false)
    {
      init = true;
      logFile = fopen("program.log", "w");
      
      if (logFile == NULL)
      {
        fprintf(stderr, "Unable to open program.log\n");
        // We have a backup plan!! Try the system temporary directory.
        std::error_code ec;
        std::string logFileName =
          (std::filesystem::temp_directory_path(ec) / "program.log").string();
        fprintf(stderr, "Trying %s\n", logFileName.c_str());
        logFile = fopen(logFileName.c_str(), "w");
        if (logFile == NULL)
        {
          fprintf(stderr, "Unable to open backup %s\n", logFileName.c_str());
        }
        else
        {
          printf("Opened log file: %s\n", logFileName.c_str());
        }
      }
      else
      {
        printf("Opened log file: program.log\n");
      }
    }
    
    // first to stdout
    va_start(ap, fmt);
    vprintf(fmt,ap);
    va_end(ap);
    
    // now to file
    if (logFile)
    {
      va_start(ap, fmt);
      vfprintf(logFile, fmt, ap);
      // flush it line-by-line so we don't miss any
      fflush(logFile);
      va_end(ap);
    }
  }
}
