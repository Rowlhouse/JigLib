//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef IMAGE_FROM_FILE_HPP
#define IMAGE_FROM_FILE_HPP

#include "jiglib.hpp"

struct tPixel
{
  unsigned char r, g, b, a;
};

// Loads an image file (PNG, JPEG, BMP, TGA etc). data is resized and populated, with the first row
// being the bottom of the image.
bool LoadImageFile(JigLib::tArray2D<tPixel> & data,
               const char * fileName);

// Writes RGB data (3 bytes per pixel, first row at the top) to a PNG file
bool SavePNGImage(const char * fileName, int width, int height, const unsigned char * rgbData);

#endif
