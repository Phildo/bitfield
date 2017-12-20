#include "bitmap.h"

#include <stdlib.h>
#include <stdio.h>
#include "math.h"

#include "pix.h"
#include "str.h"

#define READFIELD(field) ({ if(sizeof(field) != fread(&field, sizeof(byte), sizeof(field), fp)) { sprintf(err->info, "Error reading field"); return ERR; } })

static ERR_EXISTS readFileHeader(FILE *fp, BitmapFileHeader *bfh, PixErr *err)
{
  READFIELD(bfh->header_field);
  if(cmp((char *)bfh->header_field,"BM")) ERROR("File not valid Bitmap"); //can cmp because bfh ought be 0'd before use
  READFIELD(bfh->size);
  if(bfh->size <= 0) ERROR("Filesize invalid");
  READFIELD(bfh->reserved_a);
  READFIELD(bfh->reserved_b);
  READFIELD(bfh->offset);
  if(bfh->size <= 0) ERROR("Data offset invalid");

  return NO_ERR;
}

static ERR_EXISTS readDIBHeader(FILE *fp, DIBHeader *dh, PixErr *err)
{
  READFIELD(dh->header_size);
  BITMAPINFOHEADER *ih = &dh->bitmap_info_header;
  BITMAPV5HEADER *v5h = &dh->bitmap_v5_header;
  switch(dh->header_size)
  {
    case BITMAPINFOHEADER_SIZE:
    case BITMAPV5HEADER_SIZE:
      break;
    case BITMAPCOREHEADER_SIZE:
    case OS22XBITMAPHEADER_SIZE:
    case BITMAPV2INFOHEADER_SIZE:
    case BITMAPV3INFOHEADER_SIZE:
    case BITMAPV4HEADER_SIZE:
    default:
      ERROR("Unsupported DIB header (header size %d)\n",dh->header_size);
      break;
  }
  READFIELD(ih->width);
  READFIELD(ih->height);
  READFIELD(ih->nplanes);
  READFIELD(ih->bpp);
  READFIELD(ih->compression);
  if(!(ih->compression == 0 || ih->compression == 3)) ERROR("Unable to process compressed bitmaps");
  READFIELD(ih->image_size);
  if(!(ih->image_size == 0 || ih->image_size >= ih->width*abs(ih->height)*(ih->bpp/8))) ERROR("Unable to process compressed bitmaps");
  READFIELD(ih->horiz_resolution);
  READFIELD(ih->vert_resolution);
  READFIELD(ih->ncolors);
  if(ih->ncolors != 0) ERROR("Unable to process indexed bitmaps");
  READFIELD(ih->nimportantcolors);

  if(dh->header_size == BITMAPV5HEADER_SIZE && ih->compression == 3)
  {
    READFIELD(v5h->bV5RedMask);
    READFIELD(v5h->bV5GreenMask);
    READFIELD(v5h->bV5BlueMask);
    if(ih->bpp == 32) READFIELD(v5h->bV5AlphaMask);
    else v5h->bV5AlphaMask = 0;
  }

  return NO_ERR;
}

static ERR_EXISTS readColorTable(FILE *fp, byte *b, int size, PixErr *err)
{
  return NO_ERR;
}

static ERR_EXISTS readGap(FILE *fp, byte *b, int size, PixErr *err)
{
  return NO_ERR;
}

static ERR_EXISTS readPixelArray(FILE *fp, byte *b, int size, PixErr *err)
{
  if(size != fread(b, sizeof(byte), size, fp)) ERROR("Can't read specified length at offset in bitmap");
  return NO_ERR;
}

static ERR_EXISTS readColorProfile(FILE *fp, byte *b, int size, PixErr *err)
{
  return NO_ERR;
}

ERR_EXISTS readBitmap(const char *infile, Bitmap *b, PixErr *err)
{
  //INPUT
  FILE *in;
  if(!(in = fopen(infile, "r"))) ERROR("Can't open input file- %s",infile);

  InternalBitmap *simple = &b->simple;

  BitmapFileHeader *bh = &b->bitmap_file_header;
  if(!readFileHeader(in, bh, err)) { fclose(in); return ERR; }

  DIBHeader *dh = &b->dib_header;
  if(!readDIBHeader(in, dh, err)) { fclose(in); return ERR; }

  //put together useful info in reading the rest
  simple->width = dh->bitmap_info_header.width;
  simple->height = abs(dh->bitmap_info_header.height);
  simple->reversed = dh->bitmap_info_header.height < 0;
  simple->bpp = dh->bitmap_info_header.bpp;
  simple->row_w = ((simple->bpp*simple->width+31)/32)*4;
  simple->pixel_n_bytes = simple->row_w*simple->height;
  simple->offset_to_data = bh->offset;
  simple->compression = dh->bitmap_v5_header.bV5Compression;
  simple->r_mask = dh->bitmap_v5_header.bV5RedMask;
  simple->g_mask = dh->bitmap_v5_header.bV5GreenMask;
  simple->b_mask = dh->bitmap_v5_header.bV5BlueMask;
  simple->a_mask = dh->bitmap_v5_header.bV5AlphaMask;

  if(dh->bitmap_info_header.ncolors > 0)
  {
    fclose(in); ERROR("Unable to process indexed bitmaps- shouldn't have gotten here");
    byte *ct = b->color_table;
    if(!readColorTable(in, ct, 0, err)) { fclose(in); return ERR; }
  }

  //does nothing...
  byte *g = b->gap1;
  if(!readGap(in, g, 0, err)) { fclose(in); return ERR; }

  if(fseek(in, simple->offset_to_data, SEEK_SET) == -1) ERROR("Unable to read bitmap file");
  b->pixel_array = calloc(simple->pixel_n_bytes,1);
  if(!b->pixel_array) ERROR("Out of memory");
  byte *pa = b->pixel_array;
  if(!readPixelArray(in, pa, simple->pixel_n_bytes, err)) { fclose(in); return ERR; }

  //does nothing...
  g = b->gap2;
  if(!readGap(in, g, 0, err)) { fclose(in); return ERR; }

  //does nothing...
  byte *cp = b->icc_color_profile;
  if(!readColorProfile(in, cp, 0, err)) { fclose(in); return ERR; }

  fclose(in);

  return NO_ERR;
}

ERR_EXISTS writeBitField(const char *out_name, BitField *b, PixErr *err)
{
  char out_file[2048];
  sprintf(out_file,"%s.%dx%dbf",out_name,b->width,b->height);

  FILE *out;
  if(!(out = fopen(out_file, "w"))) ERROR("Can't open output file- %s",out_file);

  fwrite(b->data, sizeof(byte), b->bytes, out);

  fclose(out);
  return NO_ERR;
}

ERR_EXISTS writePixImg(const char *out_name, PixImg *img, PixErr *err)
{
  char out_file[2048];
  sprintf(out_file,"%s.%dx%dpi",out_name,img->width,img->height);

  int array_size = img->width*img->height*4;
  byte *array = calloc(array_size,1);
  if(!array) ERROR("Out of memory");

  for(int y = 0; y < img->height; y++)
  {
    for(int x = 0; x < img->width; x++)
    {
      int index = ((y*img->width)+x);
      array[index*4+0] = img->data[index].r;
      array[index*4+1] = img->data[index].g;
      array[index*4+2] = img->data[index].b;
      array[index*4+3] = img->data[index].a;
    }
  }

  FILE *out;
  if(!(out = fopen(out_file, "w"))) ERROR("Can't open output file- %s",out_file);

  fwrite(array, sizeof(byte), array_size, out);

  fclose(out);

  free(array);

  return NO_ERR;
}

byte maskMap(uint32 mask)
{
  switch(mask)
  {
    case 0xff000000: return 3; break;
    case 0x00ff0000: return 2; break;
    case 0x0000ff00: return 1; break;
    case 0x000000ff: return 0; break;
    default: return 255; break;
  }
  return 255;
}

static ERR_EXISTS dataToPix(Bitmap *b, PixImg *img, PixErr *err)
{
  byte *data = b->pixel_array;
  int roww = b->simple.row_w;

  byte rmask = 0;
  byte gmask = 0;
  byte bmask = 0;
  byte amask = 0;

  switch(b->simple.bpp)
  {
    case 32:
      rmask = 3;
      gmask = 2;
      bmask = 1;
      amask = 0;
      if(b->simple.compression == 0)
      {
        rmask = 2;
        gmask = 1;
        bmask = 0;
        amask = 3;
      }
      else if(b->simple.compression == 3)
      {
        rmask = maskMap(b->simple.r_mask);
        gmask = maskMap(b->simple.g_mask);
        bmask = maskMap(b->simple.b_mask);
        amask = maskMap(b->simple.a_mask);
      }

      if(b->simple.reversed)
      {
        for(int i = 0; i < img->height; i++)
        {
          for(int j = 0; j < img->width; j++)
          {
            img->data[(i*img->width)+j].a = (amask == 255) ? 255 : data[(i*roww)+(j*4)+amask];
            img->data[(i*img->width)+j].b = (bmask == 255) ? 255 : data[(i*roww)+(j*4)+bmask];
            img->data[(i*img->width)+j].g = (gmask == 255) ? 255 : data[(i*roww)+(j*4)+gmask];
            img->data[(i*img->width)+j].r = (rmask == 255) ? 255 : data[(i*roww)+(j*4)+rmask];
          }
        }
      }
      else
      {
        for(int i = 0; i < img->height; i++)
        {
          for(int j = 0; j < img->width; j++)
          {
            img->data[(i*img->width)+j].a = (amask == 255) ? 255 : data[((img->height-1-i)*roww)+(j*4)+amask];
            img->data[(i*img->width)+j].b = (bmask == 255) ? 255 : data[((img->height-1-i)*roww)+(j*4)+bmask];
            img->data[(i*img->width)+j].g = (gmask == 255) ? 255 : data[((img->height-1-i)*roww)+(j*4)+gmask];
            img->data[(i*img->width)+j].r = (rmask == 255) ? 255 : data[((img->height-1-i)*roww)+(j*4)+rmask];
          }
        }
      }
    break;
    case 24:
      rmask = 2;
      gmask = 1;
      bmask = 0;
      amask = 255;
      if(b->simple.compression == 3)
      {
        rmask = maskMap(b->simple.r_mask);
        gmask = maskMap(b->simple.g_mask);
        bmask = maskMap(b->simple.b_mask);
        amask = maskMap(b->simple.a_mask);
      }

      for(int i = 0; i < img->height; i++)
      {
        for(int j = 0; j < img->width; j++)
        {
          img->data[(i*img->width)+j].a = (amask == 255) ? 255 : data[( (b->simple.reversed ? i : (img->height-1-i) ) *roww)+(j*3)+bmask];
          img->data[(i*img->width)+j].b = (bmask == 255) ? 255 : data[( (b->simple.reversed ? i : (img->height-1-i) ) *roww)+(j*3)+bmask];
          img->data[(i*img->width)+j].g = (gmask == 255) ? 255 : data[( (b->simple.reversed ? i : (img->height-1-i) ) *roww)+(j*3)+gmask];
          img->data[(i*img->width)+j].r = (rmask == 255) ? 255 : data[( (b->simple.reversed ? i : (img->height-1-i) ) *roww)+(j*3)+rmask];
        }
      }
    break;
    default:
      ERROR("Error parsing unsupported bpp");
      break;
  }
  return NO_ERR;
}

ERR_EXISTS bitmapToImage(Bitmap *b, PixImg *img, PixErr *err)
{
  img->width  = b->simple.width;
  img->height = b->simple.height;
  img->data = calloc(img->width*img->height*sizeof(Pix),1);
  if(!img->data) ERROR("Out of memory");
  if(!dataToPix(b, img, err)) return ERR;
  return NO_ERR;
}

ERR_EXISTS imageToBitField(PixImg *img, BitField *b, PixErr *err)
{

  b->bytes = (int)(ceil((img->width*img->height)/8.)+0.01);
  b->width = img->width;
  b->height = img->height;
  b->data = calloc(b->bytes,1);
  if(!b->data) ERROR("Out of memory");

  for(int i = 0; i < img->height; i++)
  {
    for(int j = 0; j < img->width; j++)
    {
      int index = (i*img->width)+j;
      int byte_i = index/8;
      int bit_i = index-(byte_i*8);
      if(img->data[index].r == 0)
      {
        b->data[byte_i] |= (1 << (8-1-bit_i));
        printf("1");
      }
      else
        printf("0");
    }
    printf("\n");
  }

  return NO_ERR;
}

