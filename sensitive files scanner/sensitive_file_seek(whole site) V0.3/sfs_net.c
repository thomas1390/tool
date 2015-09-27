#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory.h>
#include <locale.h>
#include "url_split.h"
#include "tree_operate.h"
#include "mysql.h"
#include "rxp.h"
#include "usage.h"
#include "curl_func.h"
#include "seek_sensitive_file.h"

char url[2048] = "http:/";

//打印出树中的所有目录和文件，并进行处理
void
construct_a_url (PTREENODE node, long int port, int head_method,
		 int need_cookie)
{
  int z, len, n;
  char *p;
  static int deepth = 0;
  //如果这个节点有孩子
  if (node->cnChild > 0)
    {
      //处理这个节点
      sprintf (url, "%s/%s", url, node->content);
      //如果深度>=1，则避免重复打印
      if (deepth < 1)
	{
	  //printf ("%s\n", url);
	  seek_sensitive_file (node,url, port, head_method, need_cookie);
	}
      //处理这个节点的所有孩子
      for (z = 0; z < node->cnChild; z++)
	{
	  len = strlen (url);
	  sprintf (url, "%s/%s", url, node->child[z]->content);
	  //printf ("%s\n", url);
	  seek_sensitive_file (node, url, port, head_method, need_cookie);
	  memset (url + len, 0x00, 1);
	}
      //进入每个孩子，处理它
      for (z = 0; z < node->cnChild; z++)
	{
	  deepth++;
	  construct_a_url (node->child[z],port,head_method,need_cookie);
	  deepth--;
	  //如果孩子处理完毕，需要恢复先前的url
	  if (z == node->cnChild - 1)
	    {
	      n = 0;
		if (deepth > 1)
		{
		  n = deepth - 1;
		}
	      for (; n < deepth; n++)
		{
		  p = strrchr (url, '/');
		  memset (p, 0x00, 1);
		}
	    }
	  //printf ("%d[DEBUG]%s\n", deepth, url);
	}
    }
  //如果没有孩子，则不需 处理
  else
    {
    }
}

int
main (int argc, char *argv[])
{
  struct stat *buf;

  if (argc != 5)
    {
      usage (argv[0]);
    }

//confirm if the cookie.txt exist?
  if (atoi (argv[4]))
    {
      if (stat ("cookie.txt", buf))
	{
	  printf ("not found cookie.txt: %s!\n", strerror (errno));
	  exit (0);
	}
    }

  //tree define
  PTREENODE root = (PTREENODE) malloc (sizeof (TREENODE));	//根节点
  root->cnChild = 0;
  root->parent = NULL;
  memset (root->child, 0, MAXSIZE);

//sql
  char sql_str[2048] = { 0x00 };
//temporary pointer
  char *p;
  char sql_result[2048] = { 0x00 };
  char *cmp_abn = "abnormal";
  char regex[2048] = { 0x00 };
  //常数，保存数据库中的总记录数
  int total_link_num;
  //计数器:用作统计在数据库中读取的下一个url
  int y;

  //counter: how many url has matched the argv[1]
  int count_match = 0;

  //构造这个正则表达式，匹配需要我们处理的网站
  sprintf (regex, "^http://%s", argv[1]);
  //sql初始化
  setlocale (LC_ALL, "");
  sql_init ();
  sql_locale ();
  //统计url的总数
  sprintf (sql_str, "select count(*) from crawl_site");
  p = sql_fetch (sql_str);
  memcpy (sql_result, p, strlen (p));
//error handle
  if (memcmp (sql_result, cmp_abn, 8) == 0)
    {
      printf ("[SQL ERROR]:->sql_fetch(): select count(*) from crawl_site\n");
      exit (0);
    }
  else
    {
      total_link_num = atoi (sql_result);
    }
  printf ("total %d links!\n", total_link_num);
  //获取每一条url，进行处理
  for (y = 1; y <= total_link_num; y++)
    {
      memset (sql_str, 0x00, 2048);
      memset (sql_result, 0x00, 2048);
      sprintf (sql_str,
	       "select links from crawl_site where id='%d' and (ret_code =200 or ret_code is null)",
	       y);
      p = sql_fetch (sql_str);
      memcpy (sql_result, p, strlen (p));
      //error handle
      if (memcmp (sql_result, cmp_abn, 8) == 0)
	{
	  printf ("[SQL ERROR]:->sql_fetch(): %s\n", sql_str);
	}
      else
	{
	  //如果不是我们需要的网站，继续下一个循环
	  if (match (sql_result, regex) == 0)
	    {
	      continue;
	    }
	  count_match++;
	  //printf ("url: %dth -> %s\n", y, sql_result);
	  //得到一个链接，开始处理了
	  root = tree_operate (root, sql_result, count_match);
	}
    }

//sql finish
  sql_close ();

//  PrintTree (root);           //先序打印多叉树
//  found_every_element (root);
  construct_a_url (root, atol (argv[2]), atoi (argv[3]), atoi (argv[4]));

  destroyTree (root);		//后序释放多叉树节点  

  exit (0);
}
