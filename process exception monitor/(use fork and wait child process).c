#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
main (int argc, char **argv[])
{
  if (argc != 3)
    {
      printf ("usage:%s  tvf/xvf  tar_file_name\n", argv[0]);
      exit (0);
    }
  pid_t pid;

  char *command[] = { "tar", argv[1], argv[2], (char *) 0 };
  char *envp[] = { "PATH=/bin", 0 };

  //fork
  if ((pid = fork ()) == 0)
    {
/*child process*/
      printf ("This is child process %d\n", getpid ());
      execve ("/bin/tar", command, envp);
//      exit (0);
    }

  else if (pid > 0)
    {
/*parent process*/
      pid_t child_pid;
      int child_status;
      printf ("This is parent process %d\n", getpid ());

      //for extensive later,we use -1 instead of pid
      while ((child_pid = waitpid (-1, &child_status, 0)) > 0)
	{
	  if (WIFEXITED (child_status))
	    {
	      printf ("child %d terminated with exit status = %d\n",
		      child_pid, WEXITSTATUS (child_status));
	    }
	  else if (WIFSIGNALED (child_status))
	    {
	      printf
		("child %d terminated by un-catched signal,signal number is %d\n",
		 child_pid, WTERMSIG (child_status));
	    }
	  else if (WIFSTOPPED (child_status))
	    {
	      printf ("child %d is suspended, signal number is %d\n",
		      child_pid, WSTOPSIG (child_status));
	    }
	  //debug:it is need? 
	  else
	    {
	      printf ("child %d terminated abnormally\n", child_pid);
	    }
	  //judge waitpid's  return-value
	  if (errno == ECHILD)
	    {
	      printf ("no child process,%s\n", strerror (errno));
	    }
	  if (errno == EINTR)
	    {
	      printf ("waitpaid() interrupted by a signal,%s\n",
		      strerror (errno));
	    }
	  exit (0);
	}
    }
  //fork error
  else
    {
      printf ("fork error");
      exit (0);
    }
}
