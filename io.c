#include "io.h"

#include <stdlib.h>
#include <stdio.h>

ERR_EXISTS readFile(const char *infile, PixImg *img, PixErr *err)
{
  Bitmap b = {0};
  if(!readBitmap(infile, &b, err)) return ERR;

  if(!bitmapToImage(&b, img, err)) return ERR;

  return NO_ERR;
}

ERR_EXISTS writeFile(const char *outfile, PixImg *img, PixErr *err)
{
/*
  BitField b = {0};
  if(!imageToBitField(img, &b, err)) return ERR;
  if(!writeBitField(outfile, &b, err)) return ERR;
*/

  if(!writePixImg(outfile, img, err)) return ERR;

  return NO_ERR;
}

