#define num_c 9
#define num_t 5
#define num_s 8

//原始模型是对树的节点进行遍历
//对树中的每个节点进行遍历，构造vuln url
int
guess_occasional_file (PTREENODE node, char url_arg[2048], char vuln[64],
		       long int port, int head_method, int need_cookie)
{
  int m, z, len;
  char *p;
  char node_treated[64] = { 0x00 };
  char data[2048] = { 0x00 };
  len = strlen (url_arg);
  char *page[6] = { ".htm", ".html", ".asp", ".aspx", ".php", ".jsp" };
  //if node->content is page,spilt it
  for (m = 0; m < 6; m++)
    {
      p = strstr (node->content, page[m]);
      if (p)
	{
	  memcpy (node_treated, node->content, p - node->content);
	  break;
	}
    }
  if(p==NULL){
  	memcpy (node_treated, node->content, strlen (node->content));
  	}
  sprintf (data, "%s/%s%s", url_arg, node_treated, vuln);
  memset(node_treated,0x00,64);
  if (curl_mem (data, port, head_method, need_cookie) != 1)
    {
      printf ("error occur when curl_mem() , url: %s\n", data);
    }
  memset (data, 0x00, 2048);
  if (node->cnChild > 0)
    {
      for (z = 0; z < node->cnChild; z++)
	{
	  guess_occasional_file (node->child[z], url_arg, vuln, port,
				 head_method, need_cookie);
	}
    }
  return 1;
}

int
seek_sensitive_file (PTREENODE node, char url_arg[2048], long int port,
		     int head_method, int need_cookie)
{
  char *temp[num_t] = { "~", ".tmp", ".temp", ".swap", ".bak" };
  char *common[num_c] =
    { ".bak", ".zip", ".rar", ".gz", ".tar", ".tar.gz", ".Z", ".bz", ".bz2" };
  char *sensitive[num_s] =
    { ".sql", ".mdb", ".conn", ".conf", ".config", ".ini", ".xml", ".txt" };
  char *page_suffix[6] =
    { "\\.htm$", "\\.html$", "\\.jsp$", "\\.asp$", "\\.aspx$", "\\.php$" };
  char construct_url[2048] = { 0x00 };
  //temporary
  int len1;
//cycle from common 
  int m;
  int is_page = 0;

//judge url is directory or page?
  for (m = 0; m < 6; m++)
    {
      if (match (url_arg, page_suffix[m]) == 1)
	{
	  is_page = 1;
	  break;
	}
    }
  //guess directory list
  //对每个真实目录访问一遍
  if (is_page == 0)
    {
      if (curl_mem (url_arg, port, head_method, need_cookie) != 1)
	{
	  printf ("error occur when curl_mem() , url: %s\n", url_arg);
	}
    }

  len1 = strlen (url_arg);
  //guess real file's bak
  //每个真实文件，都猜解一下是否有临时文件等
  if (is_page == 1)
    {
      for (m = 0; m < num_t; m++)
	{
	  sprintf (construct_url, "%s%s", url_arg, temp[m]);
	  if (curl_mem (construct_url, port, head_method, need_cookie) != 1)
	    {
	      printf ("error occur when curl_mem() , url: %s\n",
		      construct_url);
	    }
	  memset (construct_url, 0x00, 2048);
	}
    }

//guess occasional file -- common
//对每个目录下，都根据树中的每一个节点，猜解一遍构造的common文件
  if (is_page == 0)
    {
      for (m = 0; m < num_c; m++)
	{
	  if (guess_occasional_file
	      (node, url_arg, common[m], port, head_method, need_cookie) != 1)
	    {
	      printf ("error occur when guess_occasional_file(),url:%s\n",
		      url_arg);
	    }
	}
    }


//guess occasional file -- sensitive
//对每个目录下，都猜解一遍构造的偶然文件-sensitive
  if (is_page == 0)
    {
      for (m = 0; m < num_s; m++)
	{
	  if (guess_occasional_file
	      (node, url_arg, sensitive[m], port, head_method,
	       need_cookie) != 1)
	    {
	      printf ("error occur when guess_occasional_file(),url:%s\n",
		      url_arg);
	    }
	}
    }

  return 1;
}
