/*
usage:[guest@localhost]$ ./3.9
Usage:
        ./3.9 demo-file fuzzed-file fuzzy-string crafted-string-begin repeat-times
         explain as follows:
         fuzzed-file   (file name's max value should <=32 bytes)
         fuzzy-string   (max value should <=256bytes)
         crafted-string-begin   (min is 1)
Example:
        ./3.9 demo demo_FUZZY AAAA 500 24
         this will produce a file named demo_FUZZY with 24 AAAA strings begin @500 bytes


You can see Usage.Only 1 file will be produced.
use condition:
fuzzy string's length <= 256
fuzzed-file-name's length <= 32
*/
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
void usage ();
void build_part1 ();
void build_part3 ();

main (int argc, char **argv[])
{

//demo file
  FILE *stream1;
//fuzzed file
  FILE *stream2;
  //fuzzy string
  char *fuzzy;
  //name used to be format the new-fuzzed-file's name
  char name[32];
//n is used to counter the fuzzy-string's filled-in times
  int n;
//f_len is fuzzy-string's length
  int f_len;

//zero,justify the program's input argument
  if (argc != 6)
    {
      usage (argv[0]);
    }

  //first,calculate the fuzzy-string's-length
  f_len = strlen (argv[3]);
  if (f_len > 256)
    {
      printf
	("fuzz string's length should < 256,your input is %d,PLS check.\n",
	 f_len);
      exit (0);
    }
  else
    {
      printf ("[DEBUG]: fuzzy string's length is %d\n", f_len);
    }
  fuzzy = argv[3];

//second,get the length of demo file
  struct stat buf;
  if (stat (argv[1], &buf) == -1)
    {
      printf ("stat demo file error: %s\n", strerror (errno));
      exit (0);
    }
  //justify the demo-file's length and argv[4]\argv[5]
  if (buf.st_size < atoi (argv[4])
      || buf.st_size <= (atoi (argv[4]) + f_len * atoi (argv[5]) - f_len - 1))
    {
      printf
	("may be your input is error,PLS check the crafted-string-begin and repeat-times argument\n");
      printf
	("demo-file size: %d,crafted-string-begin: %d,fuzz-end-scope: %d\n",
	 buf.st_size, atoi (argv[4]),
	 atoi (argv[4]) + f_len * atoi (argv[5]) - 1);
      exit (0);
    }

// three,open demo file
  stream1 = fopen (argv[1], "r");
  if (NULL == stream1)
    {
      printf ("fopen() demo error: %s\n", strerror (errno));
      exit (0);
    }

// fourth,produce the fuzzed-file

//begin open the fuzzed file
  sprintf (name, "%s", argv[2]);
  stream2 = fopen (name, "w");
  if (NULL == stream2)
    {
      printf ("fopen() fuzzed error: %s\n", strerror (errno));
      exit (0);
    }
//fuzzed-file == 1st-part + (2nd)fuzzed-part + 3rd-part

//produce 1st part
  build_part1 (atoi (argv[4]), stream1, stream2);

//producethe  (2nd)fuzzed part
  //copy fuzzy-string into the fuzzed-file
  for (n = 1; n <= atoi (argv[5]); n++)
    {
      //justify if residual space < fuzz-string
      if ((atoi (argv[4]) + f_len * n) > buf.st_size)
	{
	  fwrite (fuzzy,
		  (buf.st_size - atoi (argv[4]) - f_len * n + f_len + 1), 1,
		  stream2);
	  fseek (stream1,
		 (buf.st_size - atoi (argv[4]) - f_len * n + f_len + 1),
		 SEEK_CUR);
	  break;
	}
      else
	{
	  fwrite (fuzzy, f_len, 1, stream2);
	  fseek (stream1, f_len, SEEK_CUR);
	}
    }

//produce the 3rd part
  build_part3 (buf.st_size, atoi (argv[4]) + atoi (argv[5]) * f_len - 1,
	       stream1, stream2);

  //the fuzzed file is created here finally       
  fclose (stream2);
  if (buf.st_size != ftell (stream1))
    {
      printf
	("[WARNING] %s: %s file's final read offset <%d> is not equal %s file's length <%d>\n",
	 name, argv[1], ftell (stream1), argv[1], buf.st_size);
    }

  //fifth,close demo file
  fclose (stream1);
}

void
usage (char *prog)
{
  printf
    ("Usage:\n\t%s demo-file fuzzed-file fuzzy-string crafted-string-begin repeat-times\n",
     prog);
  printf
    ("\t explain as follows:\n\t fuzzed-file   (file name's max value should <=32 bytes)\n\t fuzzy-string   (max value should <=256bytes)\n\t crafted-string-begin   (min is 1)\n",
     prog);
  printf ("Example:\n\t%s demo demo_FUZZY AAAA 500 24\n", prog);
  printf
    ("\t this will produce a file named demo_FUZZY with 24 AAAA strings begin @500 bytes\n",
     prog);
  exit (0);
}

//produce the 1st part before crafted-string-begin
void
build_part1 (int low_bound, FILE * r_stream, FILE * w_stream)
{
//part is used to fill in the normal-file-part
  char part[32];
//part_counter is the part-fill's counter
  int part_counter = 1;

  memset (part, 0x0, 32);
  while ((32 * part_counter) < low_bound)
    {
      fread (part, 32, 1, r_stream);
      fwrite (part, 32, 1, w_stream);
      part_counter++;
    }
  //residual space < 32
  if ((32 * part_counter) >= low_bound)
    {
      part_counter--;
      fread (part, (low_bound - (32 * part_counter) - 1), 1, r_stream);
      fwrite (part, (low_bound - (32 * part_counter) - 1), 1, w_stream);
    }
}

//produce the 3rd part

void
build_part3 (int demo_file_size, int up_bound, FILE * r_stream,
	     FILE * w_stream)
{
//part is used to fill in the normal-file-part
  char part[32];
//part_counter is the part-fill's counter
  int part_counter;

//by here, buf.st_size is only possible >=argv[5]
  if (demo_file_size > up_bound)
    {
      part_counter = 1;
      memset (part, 0x0, 32);
      while ((32 * part_counter) < (demo_file_size - up_bound))
	{
	  fread (part, 32, 1, r_stream);
	  fwrite (part, 32, 1, w_stream);
	  part_counter++;
	}
      //residual space < 32
      if ((32 * part_counter) >= (demo_file_size - up_bound))
	{
	  part_counter--;
	  fread (part, (demo_file_size - up_bound - 32 * part_counter),
		 1, r_stream);
	  fwrite (part,
		  (demo_file_size - up_bound - 32 * part_counter), 1,
		  w_stream);
	}
    }
  else
    {
//it means thd fuzzed file is created over,so we do nothing
    }
}
