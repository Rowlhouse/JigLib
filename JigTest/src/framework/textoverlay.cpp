//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "textoverlay.hpp"
#include "graphics.hpp"
#include "resources.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

static const char * FONT_FILE = "data/fonts/Lato-Regular.ttf";
static const float BASE_FONT_SIZE = 16.0f;   // At a display scale of 1
static const float LINE_SPACING = 1.35f;     // Line height as a multiple of the font size

// The printable ASCII characters are baked into a texture
static const int FIRST_CHAR = 32;
static const int NUM_CHARS = 95;
static const int ATLAS_SIZE = 1024;

//======================================================================================================================
// tFont - the font file, and the characters baked at one size
//======================================================================================================================
struct tFont
{
  bool mTriedLoading = false;
  std::vector<unsigned char> mFileData;
  stbtt_bakedchar mChars[NUM_CHARS];
  GLuint mTexture = 0;
  float mBakedSize = 0.0f;

  bool Prepare(float size);
  float TextWidth(const std::string & text) const;
  void DrawText(const std::string & text, float x, float baseline) const;
};

static tFont sFont;

//======================================================================================================================
// Prepare - load the font if necessary, and bake it at this size if it isn't already
//======================================================================================================================
bool tFont::Prepare(float size)
{
  if (!mTriedLoading)
  {
    mTriedLoading = true;
    std::string path = GetResourcePath(FONT_FILE);
    if (FILE * file = fopen(path.c_str(), "rb"))
    {
      fseek(file, 0, SEEK_END);
      long length = ftell(file);
      fseek(file, 0, SEEK_SET);
      if (length > 0)
      {
        mFileData.resize(length);
        if (fread(&mFileData[0], 1, length, file) != (size_t) length)
          mFileData.clear();
      }
      fclose(file);
    }
    if (mFileData.empty())
      TRACE("Warning: unable to load the font %s - there will be no text\n", path.c_str());
  }
  if (mFileData.empty())
    return false;

  size = std::floor(size);
  if (mTexture && size == mBakedSize)
    return true;

  std::vector<unsigned char> bitmap(ATLAS_SIZE * ATLAS_SIZE);
  int result = stbtt_BakeFontBitmap(&mFileData[0], 0, size, &bitmap[0], ATLAS_SIZE, ATLAS_SIZE,
                                    FIRST_CHAR, NUM_CHARS, mChars);
  if (result == 0)
  {
    TRACE("Warning: unable to bake the font at size %f\n", size);
    return false;
  }

  if (!mTexture)
    glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_2D, mTexture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &bitmap[0]);
  mBakedSize = size;
  return true;
}

//======================================================================================================================
// TextWidth
//======================================================================================================================
float tFont::TextWidth(const std::string & text) const
{
  float width = 0.0f;
  for (unsigned char c : text)
  {
    if (c >= FIRST_CHAR && c < FIRST_CHAR + NUM_CHARS)
      width += mChars[c - FIRST_CHAR].xadvance;
  }
  return width;
}

//======================================================================================================================
// DrawText - y is the baseline. Expects the font texture to be bound.
//======================================================================================================================
void tFont::DrawText(const std::string & text, float x, float baseline) const
{
  glBegin(GL_QUADS);
  for (unsigned char c : text)
  {
    if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS)
      continue;
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(mChars, ATLAS_SIZE, ATLAS_SIZE, c - FIRST_CHAR, &x, &baseline, &q, 1);
    glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
    glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
    glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
    glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
  }
  glEnd();
}

//======================================================================================================================
// GetTextPanelFontSize
//======================================================================================================================
float GetTextPanelFontSize(int numLines)
{
  int w, h;
  GetWindowSize(w, h);
  // The display scale is 1 at 100%, 2 at 200% etc
  float displayScale = GetMainWindow() ? SDL_GetWindowDisplayScale(GetMainWindow()) : 1.0f;
  if (displayScale <= 0.0f)
    displayScale = 1.0f;
  float size = BASE_FONT_SIZE * displayScale;
  // Shrink it if the panel wouldn't fit
  float fitSize = h / ((numLines + 2) * LINE_SPACING);
  size = std::min(size, fitSize);
  return std::max(size, 8.0f);
}

//======================================================================================================================
// DrawTextPanel
//======================================================================================================================
void DrawTextPanel(const std::vector<std::string> & lines, int x, int y, float fontSize)
{
  if (lines.empty())
    return;
  int w, h;
  GetWindowSize(w, h);
  if (w <= 0 || h <= 0 || !sFont.Prepare(fontSize))
    return;

  const float lineHeight = std::floor(sFont.mBakedSize * LINE_SPACING);
  const float padding = std::floor(0.6f * sFont.mBakedSize);
  const float columnGap = std::floor(1.5f * sFont.mBakedSize);

  // Split into columns and measure
  std::vector<std::string> col0(lines.size()), col1(lines.size());
  float col0Width = 0.0f, col1Width = 0.0f, singleWidth = 0.0f;
  for (size_t i = 0 ; i < lines.size() ; ++i)
  {
    const std::string & line = lines[i];
    size_t tab = line.find('\t');
    col0[i] = line.substr(0, tab);
    if (tab != std::string::npos)
    {
      col1[i] = line.substr(tab + 1);
      col0Width = std::max(col0Width, sFont.TextWidth(col0[i]));
      col1Width = std::max(col1Width, sFont.TextWidth(col1[i]));
    }
    else
    {
      singleWidth = std::max(singleWidth, sFont.TextWidth(col0[i]));
    }
  }
  float panelWidth = std::max(col0Width + columnGap + col1Width, singleWidth) + 2.0f * padding;
  float panelHeight = lines.size() * lineHeight + 2.0f * padding;

  // 2D drawing in window pixels, with y down
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, w, h, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef((float) x, (float) y, 0.0f);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_FOG);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Background
  glDisable(GL_TEXTURE_2D);
  glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
  glBegin(GL_QUADS);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(panelWidth, 0.0f);
  glVertex2f(panelWidth, panelHeight);
  glVertex2f(0.0f, panelHeight);
  glEnd();

  // Text - the texture's alpha is the glyph coverage, and the colour comes from glColor
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, sFont.mTexture);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  float ascent = std::floor(0.8f * sFont.mBakedSize);
  for (size_t i = 0 ; i < lines.size() ; ++i)
  {
    float baseline = padding + i * lineHeight + ascent;
    if (!col0[i].empty() && col0[i][0] == '#')
    {
      glColor4f(1.0f, 0.85f, 0.3f, 1.0f);
      sFont.DrawText(col0[i].substr(1), padding, baseline);
      continue;
    }
    glColor4f(0.6f, 0.9f, 1.0f, 1.0f);
    sFont.DrawText(col0[i], padding, baseline);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    sFont.DrawText(col1[i], padding + col0Width + columnGap, baseline);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}
