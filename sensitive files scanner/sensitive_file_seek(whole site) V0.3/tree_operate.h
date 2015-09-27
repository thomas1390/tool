#include "tree_full_algorithm.h"
//ÿ������Ŀ¼��ҳ����
#define MAXSIZE 2048		//���Ƕ���������Եķ�֧��

//��һ��url���в�֣��γ�һ����
PTREENODE
tree_operate (PTREENODE root, char url[2048], int count_url)
{
//url split
  //dir_num:ÿ��siteĿ¼������
  //x:��ʼ��ָ������
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

  //tree�㷨��ʼ������ÿ��url���д���
  //alloc memory
  for (x = 0; x < 16; x++)
    {
      url_data.directory[x] = malloc (4096);
      url_data.parameter[x] = malloc (4096);
    }

  //������url
  result_data = split_url (url, &url_data);
  dir_num = result_data->num_directory;

  match_flag = MAXSIZE + 1;

  //������ڵ�
  if (count_url == 1)
    {
      strncpy (root->content, result_data->domain,
	       strlen (result_data->domain));
    }
  //�������Ŀ¼
  if (dir_num > 0)
    {
      //����Ŀ¼�ڵ�
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
			  //�ظ�
			  match_flag = n;
			  //skII
			  temp[m] = root->child[n];
			  break;
			}
		    }
		  if (cmp != 0)
		    //���ظ�
		    temp[m] = InsertNode (&root, result_data->directory[m]);

		}
	    }
	  else
	    {
//����ظ�
	      if (match_flag < MAXSIZE + 1)
		{
		  if (temp[m - 1]->cnChild == 0)
		    {
		      //���û����
		      temp[m] =
			InsertNode (&temp[m - 1], result_data->directory[m]);
		    }
//����к���
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
//������ظ�
		  if (temp[m - 1]->cnChild == 0)
		    {
		      //���û����
		      temp[m] =
			InsertNode (&temp[m - 1], result_data->directory[m]);
		    }
//����к���
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

//����ҳ���ڵ�
      if (result_data->page_exist)
	{
//����ظ�
	  if (match_flag < MAXSIZE + 1)
	    {
	      if (temp[m - 1]->cnChild == 0)
		{
		  //���û����
		  InsertNode (&temp[m - 1], result_data->page);
		}
//����к���
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
//������ظ�
	      if (temp[m - 1]->cnChild == 0)
		{
		  //���û����
		  InsertNode (&temp[m - 1], result_data->page);
		}
//����к���
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
  //���������Ŀ¼
  else
    {
      //����ҳ���ڵ�
      if (result_data->page_exist)
	{
	  if (root->cnChild == 0)
	    {
	      //���û����
	      InsertNode (&root, result_data->page);
	    }
//����к���
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
  //tree�㷨����
  return root;
}
