//modified from the tcp proxy - sp.c, Oct 2012
//usage: gcc -o sp sp.c
//if the scanner support proxy,you can use it to do some interesting things,enjoy yourself:)
//firstly thanks the author Alan Chen,the original notes and codes are below, except some new added functional line
/******************************************************************************* 
 This program is free software; you can redistribute it and/or modify 
 it under the terms of the GNU General Public License as published by 
 the Free Software Foundation; either version 2 of the License, or 
 (at your option) any later version. 

 This program is distributed in the hope that it will be useful, 
 but WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 GNU General Public License for more details. 

 You should have received a copy of the GNU General Public License 
 along with this program; if not, write to the Free Software 
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 *******************************************************************************/
/******************************************************************************* 
 Program: sp.c 
 Des cription: a smart proxy 
 Author: c (ariesram@may10.ca) 
 Date: July 18, 2001 
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>

#define ERRLOG "./sp.log"

int do_proxy (int infd, char *addr, int port);
int max (int i, int j);
void waitchild (int);
void version ();
void usage ();
void daemonize ();
void p_error (const char *err_msg);
int
main (int argc, char **argv)
{
  struct sockaddr_in servaddr, clientaddr;
  int listenfd, connfd;
  socklen_t clientlen;
  pid_t chpid;
  int service_port = 0, remote_port = 0;
  char remote_addr[17];
  const char optstring[] = "s:r:p:vh";
  int opt;
  extern char *optarg;
  extern int optind, opterr, optopt;
  extern FILE *stderr;
  memset (remote_addr, 0x00, 17);
  if (argc == 2)
    {
      while ((opt = getopt (argc, argv, optstring)) != -1)
	{
	  if (opt == 'v')
	    {
	      version ();
	      exit (0);
	    }
	  else if (opt == 'h')
	    {
	      usage ();
	      exit (0);
	    }
	  else
	    {
	      printf ("type sp -h for help message");
	      usage ();
	      exit (0);
	    }
	}
    }
  else
    {
      while ((opt = getopt (argc, argv, optstring)) != -1)
	{
	  switch (opt)
	    {
	    case 's':
	      service_port = atoi (optarg);
	      break;
	    case 'r':
	      memcpy (remote_addr, optarg, strlen (optarg));
	      remote_addr[strlen (remote_addr)] = 0x00;
	      break;
	    case 'p':
	      remote_port = atoi (optarg);
	      break;
	    default:
	      usage ();
	      exit (0);
	    }
	}
    }
  if (service_port == 0 || remote_port == 0 || remote_addr[0] == 0x00)
    {
      usage ();
      exit (0);
    }

  daemonize ();

  bzero (&servaddr, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons (service_port);
  servaddr.sin_addr.s_addr = INADDR_ANY;
  listenfd = socket (AF_INET, SOCK_STREAM, 0);

  if (listenfd < 0)
    {
      p_error ("socket error");
      exit (-1);
    }
  if (bind (listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) < 0)
    {
      p_error ("bind error");
      exit (-1);
    }
  if (listen (listenfd, 5) < 0)
    {
      p_error ("listen error");
      exit (-1);
    }

  signal (SIGCHLD, waitchild);

  for (;;)
    {
      connfd = accept (listenfd, (struct sockaddr *) &clientaddr, &clientlen);
      if (connfd < 0)
	{
	  p_error ("accept error");
	  exit (-1);
	}
      if ((chpid = fork ()) == -1)
	{
	  p_error ("fork error");
	  exit (-1);
	}
      if (chpid == 0)
	{
	  close (listenfd);
	  do_proxy (connfd, remote_addr, remote_port);
	  exit (0);
	}
      if (chpid > 0)
	{
	  close (connfd);
	}
    }
  exit (0);
}

//infb ->client
int
do_proxy (int infd, char *addr, int port)
{
  struct sockaddr_in rout;
  int outfd;
  int maxfd;
  int count = 65535;
  int n;
  fd_set set;
  char buf[count];
  char url[256];

  bzero (&rout, sizeof (rout));
  rout.sin_family = AF_INET;
  rout.sin_port = htons (port);
  rout.sin_addr.s_addr = inet_addr (addr);

  if ((outfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      p_error ("socket error");
      exit (-1);
    }
  if (connect (outfd, (struct sockaddr *) &rout, sizeof (rout)) < 0)
    {
      p_error ("connect error");
      exit (-1);
    }

  while (1)
    {
      FD_ZERO (&set);
      FD_SET (infd, &set);
      FD_SET (outfd, &set);
      maxfd = max (outfd, infd);
      if (select (maxfd + 1, &set, NULL, NULL, 0) < 0)
	{
	  perror ("select error:");
	  exit (-1);
	}
      if (FD_ISSET (infd, &set))
	{
	  n = read (infd, (void *) buf, count);
	  if (n <= 0)
	    break;
//new code: change the packet from scanner
	  if(0!=strstr(buf,"identifier")){
		memset(strstr(buf,"identifier"),0x74,10);
	  	}
//end	  
	  if (write (outfd, (const void *) buf, n) != n)
	    {
	      p_error ("write error");
	      continue;
	    }
	}
      if (FD_ISSET (outfd, &set))
	{
	  n = read (outfd, (void *) buf, count);
	  if (n <= 0)
	    break;

//new code: change the packet from web site
	  if (0 != strstr (buf, "disableHTTP/1.1 302 Moved Temporarily"))
	    {
	      char new_return_head[1024] ="HTTP/1.1 200 OK\r\nServer: Apache-Coyote/1.1\r\nContent-Type: text/html;charset=UTF-8\r\n";
	      char html_content[512] ="Content-Length: 21\r\n\r\n<html>ttttt</html>";
	      char *p, *q;

	      p = strstr (buf, "Date: ");
	      q = strstr (p, "\r\n");
	      strncat (new_return_head, p, q - p);
	      strncat (new_return_head, "\r\n", 2);
	      strncat (new_return_head, html_content, sizeof (html_content));

	      if (write (infd, new_return_head, sizeof (new_return_head)) !=
		  sizeof (new_return_head))
		{
		  p_error ("write error");
		  continue;
		}
	    }
//end
	  else
	    {
	      if (write (infd, (const void *) buf, n) != n)
		{
		  p_error ("write error");
		  continue;
		}
	    }
	}
    }
  close (infd);
  close (outfd);

}

int
max (int i, int j)
{
  return i > j ? i : j;
}

void
waitchild (int signo)
{

  int status;
  pid_t childpid;

  if ((childpid = waitpid (-1, &status, WNOHANG)) < 0)
    {
      p_error ("wait error");
      exit (1);
    }
  return;
}

void
version ()
{
  printf
    ("GNU SP 1.0 Copyright 2001 Aryes Software Studio, Inc. SP is free software, covered by the GNU General Public License, and you are welcome to change it and/or distribute copies of it under certain conditions.\n");
}

void
usage ()
{
  printf
    ("This is the GNU smart proxy daemon.\n"
"Usage: sp -s service_port -r remote_address -p remote_port sp -v sp -h\n"
"option: -s specify service port of sp\n"
"-r specify remote address which sp will connect\n"
"-p specify remote port while sp will connect to\n"
"-v display version message\n"
"-h display this help message\n"
"Report bugs to <ariesram@may10.ca>.\n");
}

void
daemonize ()
{

  int i;

#ifndef _DEBUG
  signal (SIGINT, SIG_IGN);

#endif /* 
        */
  signal (SIGHUP, SIG_IGN);
  signal (SIGABRT, SIG_IGN);
  signal (SIGSTOP, SIG_IGN);
  signal (SIGCHLD, SIG_IGN);

#ifndef _DEBUG
  if (fork () != 0)
    exit (0);
  setsid ();

  for (i = 256; i >= 0; i--)
#endif /* 
        */
#ifdef _DEBUG
    for (i = 256; i >= 3; i--)
#endif /* 
        */
      close (i);
}

void
p_error (const char *err_msg)
{
  FILE *fp;
#ifdef _DEBUG
  printf ("%s ", err_msg);
#endif /* 
        */
  fp = fopen (ERRLOG, "a");
  if (fp == NULL)
    return;
  fprintf (fp, "%s ", err_msg);
  fclose (fp);
}
