#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
int
match (char *str, char expr[1024])
{
  int err;
  char buf[4096];
  regex_t reg;
  regmatch_t matches[1];
  if ((err = regcomp (&reg, expr, 0)))
    {
      regerror (err, &reg, buf, 4096);
      fprintf (stderr, "%s\n", buf);
      exit (0);
    }
  if ((regexec (&reg, str, 1, matches, 0)) == 0)
    {
      //printf ("[DEBUG]:%s match,\n", str);
      regfree (&reg);
      return 1;
    }
  else
    {
      //printf ("[DEBUG:not match]\n");
      regfree (&reg);
      return 0;
    }
}
