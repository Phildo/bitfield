#include <stdio.h>  //printf, fread, fwrite
#include <stdlib.h>  //free (to please valgrind)

#include "dotypes.h"
#include "str.h"
#include "err.h"

#include "pix.h"
#include "io.h"

static char *usage = "Usage: piximg -i input_file -o outname -t [pi|bf]";
static char *default_out = "out";
static int out_type = 0; //0 = piximg, 1 = bitfield

ERR_EXISTS parseArgs(int argc, char **argv, char **infile, char **outname, PixErr *err)
{
  for(int i = 1; i < argc; i++)
  {
         if(cmp(argv[i],"-i")  == 0) *infile    = argv[++i];
    else if(cmp(argv[i],"-o")  == 0) *outname   = argv[++i];
    else if(cmp(argv[i],"-t")  == 0)
    {
      i++;
           if(cmp(argv[i],"pi") == 0) out_type = 0;
      else if(cmp(argv[i],"bf") == 0) out_type = 1;
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
  if(!parseArgs(argc, argv, &infile_str, &outname_str, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  PixImg img = {0};
  if(infile_str)
  {
    if(!readFile(infile_str, &img, &err))
    { fprintf(stderr,"%s\n",err.info); return 1; }
  }

  if(!writeFile(outname_str, &img, out_type, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  //to please valgrind
  free(img.data);

  return 0;
}

