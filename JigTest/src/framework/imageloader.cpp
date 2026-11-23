//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               danny@rowlhouse.freeserve.co.uk
//======================================================================================================================
#include "imageloader.hpp"
#include "resources.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace JigLib;

//======================================================================================================================
// LoadImage
//======================================================================================================================
bool LoadImageFile(tArray2D<tPixel> & image,
               const char * fileName)
{
  image.Resize(0, 0);

  TRACE_FILE_IF(ONCE_3)
    TRACE("Loading image: %s\n", fileName);

  int width, height, channels;
  unsigned char * data = stbi_load(GetResourcePath(fileName).c_str(), &width, &height, &channels, 4);
  if (!data)
  {
    TRACE("Error loading image %s: %s\n", fileName, stbi_failure_reason());
    return false;
  }

  // Copy into our array, flipping it so that the first row is the bottom of the image
  image.Resize(width, height);
  for (int y = 0 ; y < height ; ++y)
  {
    for (int x = 0 ; x < width ; ++x)
    {
      const unsigned char * src = data + (x + (height - y - 1) * width) * 4;
      tPixel & pixel = image(x, y);
      pixel.r = src[0];
      pixel.g = src[1];
      pixel.b = src[2];
      pixel.a = src[3];
    }
  }

  stbi_image_free(data);
  return true;
}

//======================================================================================================================
// SavePNGImage
//======================================================================================================================
bool SavePNGImage(const char * fileName, int width, int height, const unsigned char * rgbData)
{
  return stbi_write_png(fileName, width, height, 3, rgbData, width * 3) != 0;
}
