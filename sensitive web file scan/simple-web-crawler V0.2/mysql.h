//mysql
//mysql compile argument: `mysql_config --cflags --libs`
//mysq放在一个文件里，比较容易引用
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <mysql.h>
#define HOST "192.168.85.135"
#define USERNAME "sp"
#define PASSWORD "sp"
#define DATABASE "spider"

MYSQL my_connection;

MYSQL
sql_init ()
{
  int my_res;
  mysql_init (&my_connection);

  if (mysql_real_connect
      (&my_connection, HOST, USERNAME, PASSWORD, DATABASE, 0, NULL,
       CLIENT_FOUND_ROWS))
    {
      printf ("[MySQL-DEBUG]:sql_init() connection ok.\n");
      return my_connection;
    }
  else
    {
      printf ("[ERROR]:sql_init failed,reason: %s\n",
	      mysql_error (&my_connection));
      exit (0);
    }
}

int
sql_locale ()
{
  int my_res;
  /*这句话是设置查询编码为utf8，这样支持中文 */
  my_res = mysql_query (&my_connection, "set names gb2312");
  if (my_res != 0)
    {
      printf
	("[ERROR]:mysql_query failed![SQL]:set names gdb2312![REASON]: %s\n",
	 mysql_error (&my_connection));
      exit (0);
    }
  else
    {
      printf ("[MySQL-DEBUG]:sql_locale() ok.\n");
    }
}

int
sql_truncate ()
{
  int my_res;
  my_res = mysql_query (&my_connection, "truncate crawl_site");
  if (my_res != 0)
    {
      printf
	("[ERROR]:mysql_query failed![SQL]:truncate crawl_site![REASON]: %s\n",
	 mysql_error (&my_connection));
      exit (0);
    }
  else
    {
      printf ("[MySQL-DEBUG]:sql_truncate() ok.\n");
    }
}

int
sql_query (char *sql)
{
  int my_res;
  my_res = mysql_query (&my_connection, sql);
  if (my_res != 0)
    {
//注释，否则有很多重复插入失败的提示
      //printf ("[ERROR]:mysql_query failed![SQL]:%s[REASON]: %s\n", sql,mysql_error (&my_connection));
      return 0;
    }
  else
    {
      /*返回执行sql后影响的行数 */
      //printf ("%ld rows were affected!\n", (long) mysql_affected_rows(&my_connection));
      return 1;
    }
}

/*执行sql语句，返回查询结果*/
char *
sql_fetch (char *sql)
{
  int res;
  MYSQL_RES *res_ptr;		/*指向查询结果的指针 */
  MYSQL_ROW result_row;		/*按行返回的查询信息 */
  char *normal = "normal";
  char *abnormal = "abnormal";
  res = mysql_query (&my_connection, sql);
  if (res != 0)
    {
      printf ("[ERROR]:mysql_query failed![SQL]:%s[REASON]: %s\n", sql,
	      mysql_error (&my_connection));
      return abnormal;
    }
  else
    {
      res_ptr = mysql_store_result (&my_connection);
      if (res_ptr)
	{
	  result_row = mysql_fetch_row (res_ptr);
	  mysql_free_result (res_ptr);
	  if (result_row != NULL)
	    {
	      return result_row[0];
	    }
	  else
	    {
	      if (mysql_error (&my_connection)[0] != '\0')
		{
		  printf ("[ERROR]:mysql_fetch_row failed![REASON]: %s\n",
			  mysql_error (&my_connection));
		  return abnormal;
		}
	      else
		{
		  return normal;
		}
	    }
	}
      else
	{
	  printf ("[ERROR]:mysql_store_result failed![REASON]: %s\n",
		  mysql_error (&my_connection));
	  return abnormal;
	}
    }
}

int
sql_close ()
{
  mysql_close (&my_connection);
  printf ("[MySQL-DEBUG]:sql_close() connection ok.\n");
  return 1;
}
