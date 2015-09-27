//Ŀǰ����:�����64��Ŀ¼���16��������256���ڴ棬ҳ�ļ��32���������16��������256�ڴ棬����url�256
struct url_splitted
{
  char domain[64];
  char *directory[16];
  int num_directory;
  char page[64];
  int page_exist;
  char *parameter[16];
};
//�ṹ�����βΣ���ָ���һ��

struct url_splitted *
split_url (char url[2048], struct url_splitted *data)
{
  char *trim_url;
  char *p;

  data->num_directory=0;
  data->page_exist = 0;
  //www.host.com/aaa/bbb/ccc/dd.jsp?e=str1&f=str2&g=12
  trim_url = url + 7;

  //split domain
  p = strchr (trim_url, '/');
  if (!p)
    {
      //�ֶ�����0x00�ս��
      strncpy (data->domain, trim_url, strlen (trim_url) + 1);
      return data;
    }
  strncpy (data->domain, trim_url, p - trim_url);
  //�ֶ�����0x00�ս��
  memset (p - trim_url + data->domain, 0x00, 1);
  trim_url = p + 1;
  if (strlen (trim_url) == 0)
    {
      return data;
    }

  //split directory
  //aaa/bbb/ccc/dd.jsp?e=str1&f=str2&g=12
  //dd.jsp?e=str1&f=str2&g=12
  //dd.jsp
  //
  // n is the number of '/'
  for (data->num_directory = 0; data->num_directory < 16;
       ++data->num_directory)
    {
      p = strchr (trim_url, '/');
      if (p)
	{
	  strncpy (data->directory[data->num_directory], trim_url,
		   p - trim_url);
	  memset (p - trim_url + data->directory[data->num_directory], 0x00,
		  1);
	  trim_url = p + 1;
	}
      else
	break;
    }
  if (strlen (trim_url) == 0)
    {
      return data;
    }
  data->page_exist = 1;
  //split page file
  //dd.jsp?e=str1&f=str2&g=12
  p = strchr (trim_url, '?');
  if (p)
    {
      strncpy (data->page, trim_url, p - trim_url);
      memset (p - trim_url + data->page, 0x00, 1);
      //����ָ��parameter����ʱ����
      trim_url = p + 1;
    }
  else
    {
      strncpy (data->page, trim_url, strlen (trim_url) + 1);
      return data;
    }

  return data;
}
