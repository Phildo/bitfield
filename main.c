#include <stdio.h>  //printf, fread, fwrite
#include <stdlib.h>  //free (to please valgrind)

#include "dotypes.h"
#include "str.h"
#include "err.h"

#include "pix.h"
#include "io.h"

static char *usage = "Usage: pixql -i input_file -o output_file";
static char *default_out = "out.bmp";

ERR_EXISTS parseArgs(int argc, char **argv, char **infile, char **outfile, PixErr *err)
{
  for(int i = 1; i < argc; i++)
  {
         if(cmp(argv[i],"-i")  == 0) *infile    = argv[++i];
    else if(cmp(argv[i],"-o")  == 0) *outfile   = argv[++i];
  }

  if(!*infile)               ERROR("%s\nNo input file specified.",  usage);
  if(!*outfile)              *outfile = default_out; //ERROR("%s\nNo output file specified.", usage);

  return NO_ERR;
}

int main(int argc, char **argv)
{
  PixErr err = {0};

  char *infile_str = 0;
  char *outfile_str = 0;
  if(!parseArgs(argc, argv, &infile_str, &outfile_str, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  PixImg img = {0};
  if(infile_str)
  {
    if(!readFile(infile_str, &img, &err))
    { fprintf(stderr,"%s\n",err.info); return 1; }
  }

  if(!writeFile(outfile_str, &img, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  //to please valgrind
  free(img.data);

  return 0;
}

