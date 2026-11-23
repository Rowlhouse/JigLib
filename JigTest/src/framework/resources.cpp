//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "resources.hpp"

#include "jiglib.hpp"

#include <SDL3/SDL_filesystem.h>

#include <filesystem>

namespace fs = std::filesystem;

static bool IsResourceDir(const fs::path & dir)
{
  std::error_code ec;
  return fs::is_directory(dir / "configs", ec) && fs::is_directory(dir / "data", ec);
}

//======================================================================================================================
// GetResourceDir
//======================================================================================================================
const std::string & GetResourceDir()
{
  static std::string resourceDir;
  static bool initialised = false;
  if (initialised)
    return resourceDir;
  initialised = true;

  std::vector<fs::path> candidates;
  if (const char * basePath = SDL_GetBasePath())
    candidates.push_back(fs::path(basePath));
#ifdef JIGTEST_SOURCE_DIR
  candidates.push_back(fs::path(JIGTEST_SOURCE_DIR));
#endif
  candidates.push_back(fs::current_path());

  for (const fs::path & dir : candidates)
  {
    if (IsResourceDir(dir))
    {
      resourceDir = dir.string();
      TRACE("Using resources in %s\n", resourceDir.c_str());
      return resourceDir;
    }
  }

  TRACE("Warning: unable to find the configs and data folders\n");
  resourceDir = fs::current_path().string();
  return resourceDir;
}

//======================================================================================================================
// GetResourcePath
//======================================================================================================================
std::string GetResourcePath(const std::string & path)
{
  fs::path p(path);
  if (p.is_absolute())
    return path;

  std::error_code ec;
  fs::path inResources = fs::path(GetResourceDir()) / p;
  if (fs::exists(inResources, ec))
    return inResources.string();
  return path;
}

//======================================================================================================================
// GetConfigPath
//======================================================================================================================
std::string GetConfigPath(const std::string & name)
{
  fs::path configsDir = fs::path(GetResourceDir()) / "configs";
  if (name.empty())
    return (configsDir / "default.cfg").string();

  std::error_code ec;
  fs::path p(name);

  // A plain name such as "castle"
  if (!p.has_extension() && !p.has_parent_path())
  {
    fs::path named = configsDir / (name + ".cfg");
    if (fs::exists(named, ec))
      return named.string();
  }

  if (fs::exists(p, ec))
    return name;

  fs::path inConfigs = configsDir / p;
  if (fs::exists(inConfigs, ec))
    return inConfigs.string();

  return name;
}
