#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
//ҳ���ļ���������2048��
#define MAXSIZE 2048		//���Ƕ���������Եķ�֧��

typedef struct _TREENODE
{
  //�޶����256��Ŀ¼/�ļ� ��
  char content[256];
  int cnChild;
  struct _TREENODE *parent;
  struct _TREENODE *child[MAXSIZE];
} TREENODE, *PTREENODE;

PTREENODE
InsertNode (PTREENODE * ppNode, char data[256])
//��һ��������ڵ����һ�����ӽڵ�
{
  int cnChild = (*ppNode)->cnChild;
  PTREENODE temp = (PTREENODE) malloc (sizeof (TREENODE));
  strncpy (temp->content, data, strlen (data));
  temp->cnChild = 0;
  temp->parent = *ppNode;
  memset (temp->child, 0, MAXSIZE);

  (*ppNode)->child[cnChild] = temp;
  (*ppNode)->cnChild++;
  return temp;
}

void
PrintTree (PTREENODE root)	//�ݹ��ʽ�������ӡ�����
{
  static int depth = 0;
  int i;
  if (root == NULL)
    {
      return;
    }
  for (i = 0; i < depth; i++)
    {
      printf ("                ");
    }
  printf ("%s\n", root->content);
  for (i = 0; i < root->cnChild; i++)
    {
      depth++;
      PrintTree ((root->child)[i]);
      depth--;
    }
}

void
destroyTree (PTREENODE root)	//�ͷŶ�����ڵ���ռ�ڴ�
{
  int i;
  if (root == NULL)
    {
      return;
    }
  for (i = 0; i < root->cnChild; i++)
    {
      destroyTree ((root->child)[i]);
    }
  free (root);
}
