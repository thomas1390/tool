#include <stdio.h>

//url不能以http://开头
int
usage (char *prog)
{
  printf
    ("\t%s url domain-port HEAD-method(0,1) need-cookie(0,1)\n",
     prog);
  printf("\turl: used for regex,not need ^http://\n");
  printf ("\tExample:%s www.test.com 80 0 0\n", prog);
  exit (0);
}
