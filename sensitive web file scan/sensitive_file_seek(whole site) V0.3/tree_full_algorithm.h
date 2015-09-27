#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
//页面文件的深度最多2048层
#define MAXSIZE 2048		//这是多叉树最多可以的分支数

typedef struct _TREENODE
{
  //限定了最长256的目录/文件 名
  char content[256];
  int cnChild;
  struct _TREENODE *parent;
  struct _TREENODE *child[MAXSIZE];
} TREENODE, *PTREENODE;

PTREENODE
InsertNode (PTREENODE * ppNode, char data[256])
//向一个多叉树节点插入一个孩子节点
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
PrintTree (PTREENODE root)	//递归格式化先序打印多叉树
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
destroyTree (PTREENODE root)	//释放多叉树节点所占内存
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
