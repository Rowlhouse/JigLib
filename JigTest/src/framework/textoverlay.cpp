//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "textoverlay.hpp"
#include "graphics.hpp"

#include <stb_easy_font.h>

#include <algorithm>
#include <vector>

// stb_easy_font characters are about 7 pixels high, so this gives some space between lines
static const float LINE_HEIGHT = 12.0f;
static const float COLUMN_GAP = 16.0f;
static const float PADDING = 8.0f;

//======================================================================================================================
// TextWidth
//======================================================================================================================
static float TextWidth(const std::string & text)
{
  std::vector<char> buf(text.begin(), text.end());
  buf.push_back(0);
  return (float) stb_easy_font_width(&buf[0]);
}

//======================================================================================================================
// DrawText - at (x, y) in unscaled units
//======================================================================================================================
static void DrawText(const std::string & text, float x, float y)
{
  if (text.empty())
    return;
  std::vector<char> buf(text.begin(), text.end());
  buf.push_back(0);
  static std::vector<char> vertices(64 * 1024);
  int numQuads = stb_easy_font_print(x, y, &buf[0], 0, &vertices[0], (int) vertices.size());
  glVertexPointer(2, GL_FLOAT, 16, &vertices[0]);
  glDrawArrays(GL_QUADS, 0, numQuads * 4);
}

//======================================================================================================================
// GetTextPanelScale
//======================================================================================================================
float GetTextPanelScale(int numLines)
{
  int w, h;
  GetWindowSize(w, h);
  // Big enough to read, but small enough for the panel to fit in the window
  float scale = h / 450.0f;
  float fitScale = (h - 2.0f * PADDING) / ((numLines + 1) * LINE_HEIGHT);
  scale = std::min(scale, fitScale);
  return std::max(scale, 1.0f);
}

//======================================================================================================================
// DrawTextPanel
//======================================================================================================================
void DrawTextPanel(const std::vector<std::string> & lines, int x, int y, float scale)
{
  if (lines.empty())
    return;
  int w, h;
  GetWindowSize(w, h);
  if (w <= 0 || h <= 0)
    return;
  if (scale <= 0.0f)
    scale = GetTextPanelScale((int) lines.size());

  // Split into columns and measure
  std::vector<std::string> col0(lines.size()), col1(lines.size());
  float col0Width = 0.0f, col1Width = 0.0f;
  for (size_t i = 0 ; i < lines.size() ; ++i)
  {
    const std::string & line = lines[i];
    size_t tab = line.find('\t');
    col0[i] = line.substr(0, tab);
    if (tab != std::string::npos)
    {
      col1[i] = line.substr(tab + 1);
      col0Width = std::max(col0Width, TextWidth(col0[i]));
      col1Width = std::max(col1Width, TextWidth(col1[i]));
    }
  }
  float panelWidth = col0Width + COLUMN_GAP + col1Width;
  for (size_t i = 0 ; i < lines.size() ; ++i)
  {
    if (col1[i].empty())
      panelWidth = std::max(panelWidth, TextWidth(col0[i]));
  }
  float panelHeight = lines.size() * LINE_HEIGHT;

  // 2D drawing in window pixels, with y down
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, w, h, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);
  glDisable(GL_FOG);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTranslatef((float) x, (float) y, 0.0f);
  glScalef(scale, scale, 1.0f);

  // Background
  glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
  glBegin(GL_QUADS);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(panelWidth + 2.0f * PADDING / scale, 0.0f);
  glVertex2f(panelWidth + 2.0f * PADDING / scale, panelHeight + 2.0f * PADDING / scale);
  glVertex2f(0.0f, panelHeight + 2.0f * PADDING / scale);
  glEnd();

  // Text
  glEnableClientState(GL_VERTEX_ARRAY);
  float pad = PADDING / scale;
  for (size_t i = 0 ; i < lines.size() ; ++i)
  {
    float lineY = pad + i * LINE_HEIGHT;
    if (!col0[i].empty() && col0[i][0] == '#')
    {
      glColor4f(1.0f, 0.85f, 0.3f, 1.0f);
      DrawText(col0[i].substr(1), pad, lineY);
      continue;
    }
    glColor4f(0.6f, 0.9f, 1.0f, 1.0f);
    DrawText(col0[i], pad, lineY);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DrawText(col1[i], pad + col0Width + COLUMN_GAP, lineY);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopClientAttrib();
  glPopAttrib();
}
