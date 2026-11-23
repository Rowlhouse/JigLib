//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef RESOURCES_HPP
#define RESOURCES_HPP

#include <string>

// The resource folder is the one that contains configs/ and data/. It is either the folder holding
// the executable (for an installed copy) or the source folder (for a development build).
const std::string & GetResourceDir();

// Returns the path to use when opening a data file. Absolute paths are returned unchanged. Relative
// paths (e.g. "data/terrain.png") are looked up in the resource folder, and if the file isn't there
// they are returned unchanged, so they are relative to the current directory.
std::string GetResourcePath(const std::string & path);

// Works out which config file to use from what the user asked for. An empty name gives
// configs/default.cfg, and a name like "castle" gives configs/castle.cfg. Anything else is treated
// as a path to a config file, relative to the current directory or else to configs/.
std::string GetConfigPath(const std::string & name);

#endif
