//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef TEXTOVERLAY_HPP
#define TEXTOVERLAY_HPP

#include <string>
#include <vector>

// Draws a panel of text over the scene, with its top left corner at (x, y) in window pixels. Each
// line can have two columns separated by a tab, and the second columns are lined up. Lines that
// start with '#' are drawn as headings. scale = 0 picks a scale that suits the window size.
void DrawTextPanel(const std::vector<std::string> & lines, int x, int y, float scale = 0.0f);

// Returns the scale DrawTextPanel uses when scale = 0, for a panel with this many lines
float GetTextPanelScale(int numLines);

#endif
