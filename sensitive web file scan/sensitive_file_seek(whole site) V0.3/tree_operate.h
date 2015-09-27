#include "tree_full_algorithm.h"
//每层最多的目录或页面数
#define MAXSIZE 2048		//这是多叉树最多可以的分支数

//对一个url进行拆分，形成一棵树
PTREENODE
tree_operate (PTREENODE root, char url[2048], int count_url)
{
//url split
  //dir_num:每个site目录的数量
  //x:初始化指针数组
  int dir_num, x;
  struct url_splitted url_data;
  struct url_splitted *result_data;

  //tree
  PTREENODE temp[MAXSIZE];

  //some constant
  //cycle for tree
  int m, n;
  //tree's content and url's element compare
  int cmp = 0;
  //record if cmopare result is same
  int match_flag;

  //tree算法开始，对于每个url进行处理
  //alloc memory
  for (x = 0; x < 16; x++)
    {
      url_data.directory[x] = malloc (4096);
      url_data.parameter[x] = malloc (4096);
    }

  //拆分这个url
  result_data = split_url (url, &url_data);
  dir_num = result_data->num_directory;

  match_flag = MAXSIZE + 1;

  //插入根节点
  if (count_url == 1)
    {
      strncpy (root->content, result_data->domain,
	       strlen (result_data->domain));
    }
  //如果存在目录
  if (dir_num > 0)
    {
      //插入目录节点
      for (m = 0; m < dir_num; m++)
	{
	  if (m == 0)
	    {
	      if (root->cnChild == 0)
		{
		  temp[m] = InsertNode (&root, result_data->directory[m]);
		}
	      else
		{
		  for (n = 0; n < root->cnChild; n++)
		    {
		      cmp =
			strcmp (root->child[n]->content,
				result_data->directory[m]);
		      if (cmp == 0)
			{
			  //重复
			  match_flag = n;
			  //skII
			  temp[m] = root->child[n];
			  break;
			}
		    }
		  if (cmp != 0)
		    //不重复
		    temp[m] = InsertNode (&root, result_data->directory[m]);

		}
	    }
	  else
	    {
//如果重复
	      if (match_flag < MAXSIZE + 1)
		{
		  if (temp[m - 1]->cnChild == 0)
		    {
		      //如果没孩子
		      temp[m] =
			InsertNode (&temp[m - 1], result_data->directory[m]);
		    }
//如果有孩子
		  else
		    {
		      for (n = 0; n < temp[m - 1]->cnChild; n++)
			{
			  cmp =
			    strcmp (temp[m - 1]->child[n]->content,
				    result_data->directory[m]);
			  if (cmp == 0)
			    {
			      match_flag = n;
			      temp[m] = temp[m - 1]->child[n];
			      break;
			    }
			}
		      if (cmp != 0)
			temp[m] =
			  InsertNode (&temp[m - 1],
				      result_data->directory[m]);

		    }
		}
	      else
		{
//如果不重复
		  if (temp[m - 1]->cnChild == 0)
		    {
		      //如果没孩子
		      temp[m] =
			InsertNode (&temp[m - 1], result_data->directory[m]);
		    }
//如果有孩子
		  else
		    {
		      for (n = 0; n < temp[m - 1]->cnChild; n++)
			{
			  cmp =
			    strcmp (temp[m - 1]->child[n]->content,
				    result_data->directory[m]);
			  if (cmp == 0)
			    {
			      match_flag = n;
			      temp[m] = temp[m - 1]->child[n];
			      break;
			    }
			}
		      if (cmp != 0)
			temp[m] =
			  InsertNode (&temp[m - 1],
				      result_data->directory[m]);

		    }
		}
	    }
	}

//插入页名节点
      if (result_data->page_exist)
	{
//如果重复
	  if (match_flag < MAXSIZE + 1)
	    {
	      if (temp[m - 1]->cnChild == 0)
		{
		  //如果没孩子
		  InsertNode (&temp[m - 1], result_data->page);
		}
//如果有孩子
	      else
		{
		  for (n = 0; n < temp[m - 1]->cnChild; n++)
		    {
		      cmp =
			strcmp (temp[m - 1]->child[n]->content,
				result_data->page);
		      if (cmp == 0)
			{
			  match_flag = n;
			  break;
			}
		    }
		  if (cmp != 0)
		    InsertNode (&temp[m - 1], result_data->page);

		}
	    }
	  else
	    {
//如果不重复
	      if (temp[m - 1]->cnChild == 0)
		{
		  //如果没孩子
		  InsertNode (&temp[m - 1], result_data->page);
		}
//如果有孩子
	      else
		{
		  for (n = 0; n < temp[m - 1]->cnChild; n++)
		    {
		      cmp =
			strcmp (temp[m - 1]->child[n]->content,
				result_data->page);
		      if (cmp == 0)
			{
			  match_flag = n;
			  break;
			}
		    }
		  if (cmp != 0)
		    InsertNode (&temp[m - 1], result_data->page);
		}
	    }
	}
    }
  //如果不存在目录
  else
    {
      //插入页名节点
      if (result_data->page_exist)
	{
	  if (root->cnChild == 0)
	    {
	      //如果没孩子
	      InsertNode (&root, result_data->page);
	    }
//如果有孩子
	  else
	    {
	      for (n = 0; n < root->cnChild; n++)
		{
		  cmp = strcmp (root->child[n]->content, result_data->page);
		  if (cmp == 0)
		    {
		      match_flag = n;
		      break;
		    }
		}
	      if (cmp != 0)
		InsertNode (&root, result_data->page);

	    }
	}
    }

  //free memory
  for (x = 0; x < 16; x++)
    {
      free (url_data.directory[x]);
      free (url_data.parameter[x]);
    }
  //tree算法结束
  return root;
}
