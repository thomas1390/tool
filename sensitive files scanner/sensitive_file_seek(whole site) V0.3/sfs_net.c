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

//��ӡ�����е�����Ŀ¼���ļ��������д���
void
construct_a_url (PTREENODE node, long int port, int head_method,
		 int need_cookie)
{
  int z, len, n;
  char *p;
  static int deepth = 0;
  //�������ڵ��к���
  if (node->cnChild > 0)
    {
      //��������ڵ�
      sprintf (url, "%s/%s", url, node->content);
      //������>=1��������ظ���ӡ
      if (deepth < 1)
	{
	  //printf ("%s\n", url);
	  seek_sensitive_file (node,url, port, head_method, need_cookie);
	}
      //��������ڵ�����к���
      for (z = 0; z < node->cnChild; z++)
	{
	  len = strlen (url);
	  sprintf (url, "%s/%s", url, node->child[z]->content);
	  //printf ("%s\n", url);
	  seek_sensitive_file (node, url, port, head_method, need_cookie);
	  memset (url + len, 0x00, 1);
	}
      //����ÿ�����ӣ�������
      for (z = 0; z < node->cnChild; z++)
	{
	  deepth++;
	  construct_a_url (node->child[z],port,head_method,need_cookie);
	  deepth--;
	  //������Ӵ�����ϣ���Ҫ�ָ���ǰ��url
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
  //���û�к��ӣ����� ����
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
  PTREENODE root = (PTREENODE) malloc (sizeof (TREENODE));	//���ڵ�
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
  //�������������ݿ��е��ܼ�¼��
  int total_link_num;
  //������:����ͳ�������ݿ��ж�ȡ����һ��url
  int y;

  //counter: how many url has matched the argv[1]
  int count_match = 0;

  //�������������ʽ��ƥ����Ҫ���Ǵ������վ
  sprintf (regex, "^http://%s", argv[1]);
  //sql��ʼ��
  setlocale (LC_ALL, "");
  sql_init ();
  sql_locale ();
  //ͳ��url������
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
  //��ȡÿһ��url�����д���
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
	  //�������������Ҫ����վ��������һ��ѭ��
	  if (match (sql_result, regex) == 0)
	    {
	      continue;
	    }
	  count_match++;
	  //printf ("url: %dth -> %s\n", y, sql_result);
	  //�õ�һ�����ӣ���ʼ������
	  root = tree_operate (root, sql_result, count_match);
	}
    }

//sql finish
  sql_close ();

//  PrintTree (root);           //�����ӡ�����
//  found_every_element (root);
  construct_a_url (root, atol (argv[2]), atoi (argv[3]), atoi (argv[4]));

  destroyTree (root);		//�����ͷŶ�����ڵ�  

  exit (0);
}
