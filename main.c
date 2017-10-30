#include <stdio.h>  //printf, fread, fwrite
#include <stdlib.h>  //free (to please valgrind)

#include "dotypes.h"
#include "str.h"
#include "err.h"

#include "pix.h"
#include "bitmap.h"

static char *usage = "Usage: piximg [-r] -i input_file -o outname -t [pi|bf]";
static char *default_out = "out";

//file_type 0 = piximg, 1 = bitfield

ERR_EXISTS parseArgs(int argc, char **argv, int *file_type, int *reverse, char **infile, char **outname, PixErr *err)
{
  *reverse = 0;
  for(int i = 1; i < argc; i++)
  {
    if(i == 1 && cmp(argv[i],"-r") == 0) *reverse = 1;
    else if(     cmp(argv[i],"-i") == 0) *infile  = argv[++i];
    else if(     cmp(argv[i],"-o") == 0) *outname = argv[++i];
    else if(     cmp(argv[i],"-t") == 0)
    {
      i++;
      //file_type 0 = piximg, 1 = bitfield
           if(cmp(argv[i],"pi") == 0) *file_type = 0;
      else if(cmp(argv[i],"bf") == 0) *file_type = 1;
    }
  }

  if(!*infile)  ERROR("%s\nNo input file specified.",  usage);
  if(!*outname) *outname = default_out; //ERROR("%s\nNo output file specified.", usage);

  return NO_ERR;
}

int main(int argc, char **argv)
{
  PixErr err = {0};

  char *infile_str = 0;
  char *outname_str = 0;
  int reverse = 0;
  //file_type 0 = piximg, 1 = bitfield
  int file_type = 0;
  if(!parseArgs(argc, argv, &file_type, &reverse, &infile_str, &outname_str, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  PixImg img = {0};
  if(reverse)
  {
    if(file_type == 0)
    {
      if(!readPixImg(infile_str, &img, &err)) { fprintf(stderr,"%s\n",err.info); return 1; }
    }
    else
    {
      BitField b = {0};
      if(!readBitField(infile_str, &b, &err)) { fprintf(stderr,"%s\n",err.info); return 1; }
      if(!bitFieldToImage(&b, &img, &err))    { fprintf(stderr,"%s\n",err.info); return 1; }
    }

    if(outname_str)
    {
      Bitmap b = {0};
      if(!imageToBitmap(&img, &b, &err))      { fprintf(stderr,"%s\n",err.info); return 1; }
      if(!writeBitmap(outname_str, &b, &err)) { fprintf(stderr,"%s\n",err.info); return 1; }
    }
  }
  else //!reverse
  {
    if(infile_str)
    {
      Bitmap b = {0};
      if(!readBitmap(infile_str, &b, &err)) { fprintf(stderr,"%s\n",err.info); return 1; }
      if(!bitmapToImage(&b, &img, &err))    { fprintf(stderr,"%s\n",err.info); return 1; }
    }

    if(file_type == 0)
    {
      if(!writePixImg(outname_str, &img, &err)) { fprintf(stderr,"%s\n",err.info); return 1; }
    }
    else
    {
      BitField b = {0};
      if(!imageToBitField(&img, &b, &err))      { fprintf(stderr,"%s\n",err.info); return 1; }
      if(!writeBitField(outname_str, &b, &err)) { fprintf(stderr,"%s\n",err.info); return 1; }
    }
  }

  //to please valgrind
  free(img.data);

  return 0;
}

