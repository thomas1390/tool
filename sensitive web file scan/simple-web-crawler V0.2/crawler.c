/*compile platform:
mysql-5.0.77-4.el5_5.4
mysql-server-5.0.77-4.el5_5.4
mysql-devel-5.0.77-4.el5_5.4

glibc-2.5-49.el5_5.6
glibc-common-2.5-49.el5_5.6
glibc-devel-2.5-49.el5_5.6
glibc-headers-2.5-49.el5_5.6

curl-7.15.5-9.el5
curl-devel-7.15.5-9.el5

streamhtmlparser-0.1

*/

//streamhtmlparser regex
//now only find the html content,future may will support javascript
#include <stdio.h>
#include <streamhtmlparser/htmlparser.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
//header file
#include "mysql.h"
#include "rxp.h"
#include "curl_mem.h"
#define BUF_SIZE 2048
#define HTMLPARSER_ERROR 0

//site_name:only support 64 length scan target
char site_name[64] = { 0x00 };

//count_links : how many links have benn found and insert into database
int count_links = 1;
//count_curling:how many links have been visited
int count_curling = 1;
//count_visited:visited the matched pages
int count_visited = 0;
//record if sql_init() has executed?
int sql_flag = 0;

int
usage (char *p)
{
  printf ("\t[Usage]:%s url domain-port regex[scan_goal] 0,1(need-cookie?0,1)\n\n", p);
  printf
    ("\t[Example]:%s 192.168.85.5/manual/admin.php 8080 192.168.85.5/manual 0\n\n",
     p);
  printf
    ("\tIt will try to crawl from the url[192.168.85.5/manual/admin.php],the crawled urls must match the regex[192.168.85.5/manual].This programe don't use cookie file,and will try to resume last scan from resume.txt.\n\n");
  printf
    ("\tThis version now don't support post user/pass form,need copy the cookie.txt && creat resume.txt manually.\n");
  exit (0);
}
struct resume
{
  char name[64];
  int split1;
  int total_num;
  int split2;
  int curling_num;
  int split3;
  int curling_visited;
  int split4;
  int done;
  int end;
};

//not used now
int
empty_file (char *s)
{
  FILE *fp;
  fp = fopen (s, "w");
  if (fp == NULL)
    {
      printf ("fopen() error:%s\n", strerror (errno));
      return 0;
    }
  if (fclose (fp))
    {
      printf ("fclose() error:%s\n", strerror (errno));
      return 2;
    }
  return 1;
}

int
check_resume_point (char *s, char *name)
{
  FILE *fp;
  struct resume read_data;
  struct stat buf;
  fp = fopen (s, "r");
  if (fp == NULL)
    {
      printf ("fopen() error:%s\n", strerror (errno));
      exit (0);
    }
  else
    {
      if (stat (s, &buf))
	{
	  printf ("stat() error:%s\n", strerror (errno));
	  exit (0);
	}
      if ((off_t) buf.st_size == 0)
	{
	  printf ("no resume-point.\n");
	}
      else
	{
	  if (1 != fread (&read_data, sizeof (struct resume), 1, fp))
	    {
	      printf ("fread() error!\n");
	      printf ("we will reset count_curling=1\n");
	    }
	  else
	    {
	      if (strcmp (read_data.name, name))
		{
		  printf ("resume.txt's content is not %s\n", name);
		  printf
		    ("we will truncate resume.txt,and reset count_curling=1\n");
		}
	      else
		{
		  if (read_data.done)
		    {
		      printf ("this domain has been scanned finished!\n");
		      exit (0);
		    }
		  else
		    {
		      count_links = read_data.total_num;
		      printf ("count_links == %d\n", count_links);
		      count_curling = read_data.curling_num;
		      printf ("count_curling == %d\n", count_curling);
		      count_visited = read_data.curling_visited;
		      printf ("count_visited == %d\ndone!\n\n",
			      count_visited);
		    }
		}
	    }
	}
    }
  if (fclose (fp))
    {
      printf ("fclose() error:%s\n", strerror (errno));
      exit (0);
    }
}

//0x23==#
//0x0a==\n
int
write_resume_file (char *s, char a[64], int b, int c, int d, int e, int f,
		   int g, int h, int i, int j)
{
  FILE *fp;
  struct resume write_data;
  fp = fopen (s, "w");
  if (fp == NULL)
    {
      printf ("fopen() error:%s\n", strerror (errno));
    }
  else
    {
      //这里不太准，因为不一定什么时候按下ctrl-c，即接收到信号
      printf
	("Begin to save the resume-point.........writing %d#%d#%d into resume.txt........",
	 c, e, g);
      memset (&write_data, 0x00, sizeof (struct resume));
      strncpy (write_data.name, a, strlen (a));	//strncpy (write_data.name, site_name, strlen (site_name));
      write_data.split1 = b;
      write_data.total_num = c;
      write_data.split2 = d;
      write_data.curling_num = e;
      write_data.split3 = f;
      write_data.curling_visited = g;
      write_data.split4 = h;
      write_data.done = i;
      write_data.end = j;
      if (1 != fwrite (&write_data, sizeof (struct resume), 1, fp))	//struct resume is 4bytes align???
	{
	  printf ("fwrite() error!\n");
	}
      else
	{
	  printf ("done!\n");
	}
    }
  if (fclose (fp))
    {
      printf ("fclose() error:%s\n", strerror (errno));
    }
}

typedef void (*sighandler_t) (int);
sighandler_t
handler (int m)
{
  if (m == SIGINT)
    {
      printf ("User press Ctrl-c,abort...\n");
    }
  if (m == SIGKILL)
    {
      printf ("Killed\n");
    }
  if (m == SIGSEGV)
    {
      printf ("Program received signal SIGSEGV, Segmentation fault.\n");
    }
  if (m == 1000)
    {
      printf ("curl_mem() exit.....\n");
    }
  if (sql_flag == 1)
    {
      if (sql_close () != 1)
	{
	  printf ("sql_close failure!\n");
	}
    }
  write_resume_file ("resume.txt", site_name, 0x23, count_links, 0x23,
		     count_curling, 0x23, count_visited, 0x23, 0x00, 0x0a);
  exit (0);
}

//html_mem:actually it's the chunk.memory,contain all the html content
//current_link:the link now is dealed with,complemant the pwd-direcoty
int
prm (char *html_mem, char *current_link, int depth_num)
{
//  unsigned int getchar_ret;
  char c;
  //flag_d:if double quotation appear
  //flag_s:if single quotation appear
  int flag_d = 0, flag_s = 0;
  int ret;
  //hyper link max length:2048
  char links[BUF_SIZE] = { 0x00 };
  //current patch
  char current_path[BUF_SIZE] = { 0x00 };
  //temporary pointer
  char *p;
  //make a normal url
  char url[BUF_SIZE] = { 0x00 };
//every types of file type
//deal with:../ ./ ? / # ..
//discard:+ javascript .js .jpg  .gif .png .bmp .exe .zip .rar .7z .tar .gz .tgz .Z .doc .docx .mp3 .mp4 .wmv .wma .asf .3gp .rm .rmvb
//normal:http https
  //file still need be judged in http-response-header and here we don't consider the excluded path
  char *expr_discard[54] =
    { "^news://", "^javascript", "\\.css$", "\\.js$", "^#", "^&", "^mailto:",
    "\\.iso$",
    "\\.rpm$",
    "\\.txt$",
    "\\.exe$", "\\.doc$", "\\.docx$", "\\.ppt$", "\\.pptx$", "\\.xls$",
    "\\.xlsx$", "\\.vsd$",
    "\\.pdf$", "\\.mdb$",
    "\\.mp3$", "\\.m3u$", "\\.mp4$", "\\.mpeg$", "\\.wmv$", "\\.wmf$",
    "\\.pps$", "\\.ps$",
    "\\.pdf$", "\\.chm$",
    "\\.rtf$", "\\.csv$", "\\.wma$", "\\.asf$", "\\.asx$", "\\.3gp$",
    "\\.rm$", "\\.rmvb$",
    "\\.avi$", "\\.mov$",
    "\\.midi$", "\\.jpg$", "\\.gif$", "\\.tif$", "\\.png$", "\\.jpeg$",
    "\\.bmp$", "\\.zip$",
    "\\.7z$", "\\.tar$",
    "\\.gz$", "\\.tgz$", "\\.Z$", "\\.cab$"
  };
  char *expr_normal[4] = { "^http:", "^https:", "^ http:", "^ https:" };
  char *expr_make_slash[3] = { "^/", "^../", "^./" };
  char *expr_make_ns[1] = { "^[0-9a-zA-Z]" };
  //m:be used to cycle
  //n:be used to cycle expr[7]
  //match_flag:be used to record if match
  int m = 0, n, match_flag;
//cycle for html_mem
  int i;
  //tag:save htmlparser_tag()'s return-value
  const char *tag = "abcdefghijklmno";
  char sql_str[2048] = { 0x00 };

  htmlparser_ctx *parser = htmlparser_new ();
  depth_num++;
  for (i = 0; i < strlen (html_mem); i++)
    {
      c = *(html_mem + i);
      //printf("%c\n",c);
      ret = htmlparser_parse_chr (parser, c);
      if (!ret)
	{
	  //used for debug
	  //printf("[DEBUG]:htmlparser_parse_chr error!\n");
	  htmlparser_reset (parser);
	}
      if (htmlparser_is_url_start (parser))
	{
	  //parse tag
	  if (tag = htmlparser_tag (parser))
	    {
	      if (strncmp (tag, "a", 1) == 0 || strncmp (tag, "form", 4) == 0
		  || strncmp (tag, "iframe", 6) == 0)
		{
		  //printf ("tag=%s c=%c\n", tag, c);
		  //parse attr
		  if (htmlparser_attr (parser))
		    {
		      //printf ("attr=%s c=%c\n", htmlparser_attr (parser), c);
		    }
		  else
		    {
		      printf ("attr not found\n");
		    }
		}
	    }
	  else
	    {
	      printf ("tag not found\n");
	    }
	  //if double quotation
	  if (strncmp (&c, "\"", 1) == 0)
	    {
	      flag_d = 1;
	      continue;
	    }
	  //if single quotation
	  if (strncmp (&c, "\'", 1) == 0)
	    {
	      flag_s = 1;
	      continue;
	    }
	}
      if (strncmp (tag, "a", 1) == 0 || strncmp (tag, "form", 4) == 0
	  || strncmp (tag, "iframe", 6) == 0)
	{
	  //deal with doubale quotation
	  if (c != '\"' && flag_d == 1)
	    {
	      memset (links + m, c, 1);
	      m++;
	      continue;
	    }
	  if (flag_d == 1 && c == '\"')
	    {
	      flag_d = 0;
	      //            1.links build ok
	      // and  2.put regexp code here
	      for (n = 0; n < 54; n++)
		{
		  match_flag = 0;
		  if (match (links, expr_discard[n]))
		    {
		      //printf ("match expr_discard [%s]:%s\n", expr_discard[n],links);
		      match_flag = 1;
		      break;
		    }
		}
	      for (n = 0; n < 3; n++)
		{
		  if (match_flag == 1)
		    {
		      break;
		    }
		  match_flag = 0;
		  if (match (links, expr_make_slash[n]))
		    {
		      //printf ("match expr_make_slash [%s]:%s\n", expr_make[n],links);
		      //path convert
		      p = strrchr (current_link, '/');
		      memcpy (current_path, current_link, p - current_link);
		      if (n == 1)
			{
			  /*convert to absolute path 
			     http://www.hzcnc.com/aaa      /       ../bbb/c.htm
			     http://www.hzcnc.com/bbb/c.htm
			   */
			  p = strrchr (current_path, '/');
			  memset (p, 0x00, 1);
			  p = strchr (links, '/');
			  sprintf (url, "%s%s", current_path, p);
			}
		      else if (n == 2)
			{
			  sprintf (url, "%s%s", current_path, links + 1);
			}
		      else
			{
			  sprintf (url, "%s%s", current_path, links);
			}
		      //printf ("after added,url is:%s", url);
		      //deal with the situation:test.htm#content 
		      if (p = strchr (url, '#'))
			{
			  bzero (p, 1);
			}
		      count_links++;
		      sprintf (sql_str,
			       "insert into crawl_site set id='%d',links='%s',depth='%d'",
			       count_links, url, depth_num);
		      if (sql_query (sql_str))
			{
			  //printf ("insert into database success.\n");
			}
		      else
			{
			  count_links--;
			  //printf ("insert into database failure.\n");
			}
		      match_flag = 1;
		      memset (current_path, 0x00, BUF_SIZE);
		      break;
		    }
		}
	      for (n = 0; n < 4; n++)
		{
		  if (match_flag == 1)
		    {
		      break;
		    }
		  match_flag = 0;
		  if (match (links, expr_normal[n]))
		    {
		      //printf ("match expr_normal [%s]:%s\n", expr_normal[n],links);
		      //deal with the situation:test.htm#content 
		      if (p = strchr (links, '#'))
			{
			  bzero (p, 1);
			}
		      count_links++;
		      sprintf (sql_str,
			       "insert into crawl_site set id='%d',links='%s',depth='%d'",
			       count_links, links, depth_num);
		      if (sql_query (sql_str))
			{
			  //printf ("insert into database success.\n");
			}
		      else
			{
			  count_links--;
			  //printf ("insert into database failure.\n");
			}
		      match_flag = 1;
		      break;
		    }
		}
	      for (n = 0; n < 1; n++)
		{
		  if (match_flag == 1)
		    {
		      break;
		    }
		  match_flag = 0;
		  if (match (links, expr_make_ns[n]))
		    {
		      //printf ("match expr_make_slash [%s]:%s\n", expr_make[n],links);
		      p = strrchr (current_link, '/');
		      memcpy (current_path, current_link, p - current_link);
		      sprintf (url, "%s/%s", current_path, links);
		      //printf ("after added,url is:%s", url);
		      //deal with the situation:test.htm#content 
		      if (p = strchr (url, '#'))
			{
			  bzero (p, 1);
			}
		      count_links++;
		      sprintf (sql_str,
			       "insert into crawl_site set id='%d',links='%s',depth='%d'",
			       count_links, url, depth_num);
		      if (sql_query (sql_str))
			{
			  //printf ("insert into database success.\n");
			}
		      else
			{
			  count_links--;
			  //printf ("insert into database failure.\n");
			}
		      match_flag = 1;
		      memset (current_path, 0x00, BUF_SIZE);
		      break;
		    }
		}
	      if (match_flag == 0)
		{
		  printf ("not match any:%s\n", links);
		}
	      m = 0;
	      memset (links, 0x00, BUF_SIZE * sizeof (char));
	      memset (url, 0x00, BUF_SIZE * sizeof (char));
	      memset (sql_str, 0x00, 2048 * sizeof (char));
	    }
	  //deal with single quotation
	  if (c != '\'' && flag_s == 1)
	    {
	      memset (links + m, c, 1);
	      m++;
	      continue;
	    }
	  if (flag_s == 1 && c == '\'')
	    {
	      flag_s = 0;
	      //            1.links build ok
	      // and  2.put regexp code here
	      for (n = 0; n < 54; n++)
		{
		  match_flag = 0;
		  if (match (links, expr_discard[n]))
		    {
		      //printf ("match expr_discard [%s]:%s\n", expr_discard[n],links);
		      match_flag = 1;
		      break;
		    }
		}
	      for (n = 0; n < 3; n++)
		{
		  if (match_flag == 1)
		    {
		      break;
		    }
		  match_flag = 0;
		  if (match (links, expr_make_slash[n]))
		    {
		      //printf ("match expr_make [%s]:%s\n", expr_make[n],links);
		      p = strrchr (current_link, '/');
		      memcpy (current_path, current_link, p - current_link);
		      if (n == 1)
			{
			  /*convert to absolute path 
			     http://www.hzcnc.com/aaa      /       ../bbb/c.htm
			     http://www.hzcnc.com/bbb/c.htm
			   */
			  p = strrchr (current_path, '/');
			  memset (p, 0x00, 1);
			  p = strchr (links, '/');
			  sprintf (url, "%s%s", current_path, p);
			}
		      else if (n == 2)
			{
			  sprintf (url, "%s%s", current_path, links + 1);
			}
		      else
			{
			  sprintf (url, "%s%s", current_path, links);
			}
		      //printf ("after added,url is:%s", url);
		      //deal with the situation:test.htm#content 
		      if (p = strchr (url, '#'))
			{
			  bzero (p, 1);
			}
		      count_links++;
		      sprintf (sql_str,
			       "insert into crawl_site set id='%d',links='%s',depth='%d'",
			       count_links, url, depth_num);
		      if (sql_query (sql_str))
			{
			  //printf ("insert into database success.\n");
			}
		      else
			{
			  count_links--;
			  //printf ("insert into database failure.\n");
			}
		      match_flag = 1;
		      memset (current_path, 0x00, BUF_SIZE);
		      break;
		    }
		}
	      for (n = 0; n < 4; n++)
		{
		  if (match_flag == 1)
		    {
		      break;
		    }
		  match_flag = 0;
		  if (match (links, expr_normal[n]))
		    {
		      //printf ("match expr_normal [%s]:%s\n", expr_normal[n],links);
		      //deal with the situation:test.htm#content 
		      if (p = strchr (links, '#'))
			{
			  bzero (p, 1);
			}
		      count_links++;
		      sprintf (sql_str,
			       "insert into crawl_site set id='%d',links='%s',depth='%d'",
			       count_links, links, depth_num);
		      if (sql_query (sql_str))
			{
			  //printf ("insert into database success.\n");
			}
		      else
			{
			  count_links--;
			  //printf ("insert into database failure.\n");
			}
		      match_flag = 1;
		      break;
		    }
		}
	      for (n = 0; n < 1; n++)
		{
		  if (match_flag == 1)
		    {
		      break;
		    }
		  match_flag = 0;
		  if (match (links, expr_make_ns[n]))
		    {
		      //printf ("match expr_make [%s]:%s\n", expr_make[n],links);
		      p = strrchr (current_link, '/');
		      memcpy (current_path, current_link, p - current_link);
		      sprintf (url, "%s/%s", current_path, links);
		      //printf ("after added,url is:%s", url);
		      //deal with the situation:test.htm#content
		      if (p = strchr (url, '#'))
			{
			  bzero (p, 1);
			}
		      count_links++;
		      sprintf (sql_str,
			       "insert into crawl_site set id='%d',links='%s',depth='%d'",
			       count_links, url, depth_num);
		      if (sql_query (sql_str))
			{
			  //printf ("insert into database success.\n");
			}
		      else
			{
			  count_links--;
			  //printf ("insert into database failure.\n");
			}
		      match_flag = 1;
		      memset (current_path, 0x00, BUF_SIZE);
		      break;
		    }
		}
	      if (match_flag == 0)
		{
		  printf ("not match any:%s\n", links);
		}
	      m = 0;
	      memset (links, 0x00, BUF_SIZE * sizeof (char));
	      memset (url, 0x00, BUF_SIZE * sizeof (char));
	      memset (sql_str, 0x00, 2048 * sizeof (char));
	    }
	}
      flag_d = 0;
      flag_s = 0;
    }
  return 1;
}

int
main (int argc, char *argv[])
{
  long int port = 80;
  char link[2048] = { 0x00 };
  char *p;
  char regex[128];
  char sql_str[2048] = { 0x00 };
  char *cmp_n = "normal";
  char *cmp_abn = "abnormal";
  int end_flag = 0;
//retry:counter how many times the failed curl_mem() been executed
  int retry = 0;

//signal handle
  signal (SIGINT, handler);
  signal (SIGSEGV, handler);

  if (argc != 5)
    {
      usage (argv[0]);
    }

//justify resume point
  strncpy (site_name, argv[1], strlen (argv[1]));
  printf ("Check the resume-point...\n");
  check_resume_point ("resume.txt", argv[1]);

  port = atol (argv[2]);
  //make regex match
  sprintf (regex, "^http://%s", argv[3]);

  //sql initialize
  setlocale (LC_ALL, "");
  sql_init ();
  sql_flag = 1;
  sql_locale ();

  if (count_curling == 1)
    {
      sql_truncate ();
      sprintf (link, "http://%s/", argv[1]);
      sprintf (sql_str,
	       "insert into crawl_site set id='%d',links='%s',depth=1",
	       count_links, link);
      if (sql_query (sql_str) == 0)
	{
	  printf ("The 1st sql_query() failed,so we exit!\n");
	  if (sql_close () != 1)
	    {
	      printf ("sql_close failure!\n");
	    }
	  exit (0);
	}
      count_curling--;
    }
  else
    {
      count_curling--;
    }

  while (1)
    {
      //judge if it is a authorized site?
      do
	{
	  count_curling++;
	  memset (sql_str, 0x00, 2048);
	  sprintf (sql_str, "select links from crawl_site where id='%d'",
		   count_curling);
	  p = sql_fetch (sql_str);
	  memset (link, 0x00, 2048 * sizeof (char));
	  memcpy (link, p, strlen (p));
	  if (memcmp (link, cmp_n, 6) == 0)
	    {
	      printf
		("programe normal over!total insert %d links, and %d pages is crawled over!\n",
		 count_links, count_visited);
	      write_resume_file ("resume.txt", site_name, 0x23, count_links,
				 0x23, count_curling, 0x23, count_visited,
				 0x23, 0x01, 0x0a);
	      end_flag = 1;
	      break;
	    }
	  if (memcmp (link, cmp_abn, 8) == 0)
	    {
	      printf
		("programe abnormal over!total insert %d links, and %d pages is successfully crawled over!\n",
		 count_links, count_visited);
	      end_flag = 1;
	      break;
	    }
	}
      while (match (link, regex) == 0);
      if (end_flag == 1)
	{
	  break;
	}
      printf ("%dth link is %s\n", count_curling, link);
      if (curl_mem (link, port, count_curling,atoi(argv[4])) == 1)
	{
	  printf ("%dth link's curl_mem() executed over!\n", count_curling);
	  count_visited++;
	  retry = 0;
	}
      else
	{
	  printf
	    ("%dth link's curl_mem() executed failed,we will re-execute this link two times!\n",
	     count_curling);
	  retry++;
	  if (retry < 3)
	    {
	      count_curling--;
	    }
	}
    }
  if (sql_close () != 1)
    {
      printf ("sql_close failure!\n");
    }
}
