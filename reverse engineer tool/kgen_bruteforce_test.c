#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
main ()
{
  char s[5] = { '*', '+', ',', '-', '/' };
  char *q;

  char o[4];
  char *p;
  p = o;

//execve
  int a, b, c;
  int fd;
  char *command[] = { "/home/ttt/cm1", (char *) 0 };
  char *envp[] = { "PATH=/bin", 0 };

//fork
  pid_t pid, child_pid;
  int child_status;

  for (a = 0; a < 5; a++)
    {
      memset (p, '\0', 4);
      q = s + a;
      memcpy (p, q, 1);
      p++;
      for (b = 0; b < 5; b++)
	{
	  q = s + b;
	  memcpy (p, q, 1);
	  p++;
	  for (c = 0; c < 5; c++)
	    {
	      q = s + c;
	      memcpy (p, q, 1);
	      fd =
		open ("/home/ttt/.key_ttt",
		      O_CREAT | O_WRONLY | O_TRUNC);
	      write (fd, o, 3);
	      close (fd);
	      //fork
	      if ((pid = fork ()) == 0)
		{
		  /*child process */
		  execve ("/home/ttt/cm1", command, envp);
		}
	      else if (pid > 0)
		{
		  /*parent process */
		  child_pid = wait (&child_status);
		  if (WIFEXITED (child_status))
		    {
		      printf
			("child %d terminated with exit status = %d\n",
			 child_pid, WEXITSTATUS (child_status));
		    }
		}
	      else
		{
		  /*fork error */
		  printf
		    ("fork error,a=%d,b=%d,c=%d,d=%d,e=%d\n", a, b, c);
		  exit (0);
		}
	      //fork end
	    }
	  p--;
	}
      p--;
    }
}
