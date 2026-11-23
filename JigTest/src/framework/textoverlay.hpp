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
// start with '#' are drawn as headings. The text uses the font in data/fonts, at fontSize pixels.
void DrawTextPanel(const std::vector<std::string> & lines, int x, int y, float fontSize);

// Returns a font size (in pixels) that's comfortable to read on this display - about 16 points -
// but small enough for a panel with this many lines to fit in the window.
float GetTextPanelFontSize(int numLines);

#endif
