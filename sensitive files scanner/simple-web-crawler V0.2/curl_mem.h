#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

extern sighandler_t handler(int m);

struct MemoryStruct
{
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback (void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *) data;

  mem->memory = realloc (mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL)
    {
      /* out of memory! */
      printf ("not enough memory (realloc returned NULL)\n");
      exit (EXIT_FAILURE);
    }

  memcpy (&(mem->memory[mem->size]), ptr, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

//return the http response code
size_t
get_retcode_func (void *ptr, size_t size, size_t nmemb, void *stream)
{
  int r;
  int code = 0;

  r = sscanf (ptr, "HTTP/0.9 %d OK", &code);
  if (r)
    *((int *) stream) = code;

  r = sscanf (ptr, "HTTP/1.0 %d OK", &code);
  if (r)
    *((int *) stream) = code;

  r = sscanf (ptr, "HTTP/1.1 %d OK", &code);
  if (r)
    *((int *) stream) = code;

  return size * nmemb;
}

//link:link be dealing with
//port:domain's port
//count:which page now is been curl_mem()
int
curl_mem (char *link, long int port, int count,int cookie_need)
{
  CURL *curl_handle;
  char err_buff[CURL_ERROR_SIZE];
  CURLcode ret;
  char sql_str[2048];
  int code;
  char *depth = "empty";
  char *cmp_abn = "abnormal";

  struct MemoryStruct chunk;

  chunk.memory = malloc (1);	/* will be grown as needed by the realloc above */
  chunk.size = 0;		/* no data at this point */

  curl_global_init (CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init ();

  /* specify URL to get */
  curl_easy_setopt (curl_handle, CURLOPT_URL, link);

  /* specify a port to get */
  curl_easy_setopt (curl_handle, CURLOPT_PORT, port);

  /* support gzip algorithm */
  curl_easy_setopt (curl_handle, CURLOPT_ENCODING, "");

  //follow 302 response code
  curl_easy_setopt (curl_handle, CURLOPT_FOLLOWLOCATION, 1);

  //set dns cache time-out && also need net start nscd
  curl_easy_setopt (curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, -1);
  //set CONNECTTIMEOUT
  curl_easy_setopt (curl_handle, CURLOPT_CONNECTTIMEOUT, 10);

  if(cookie_need){ 
  curl_easy_setopt (curl_handle, CURLOPT_COOKIEFILE, "cookie.txt");
  	}

  /* send all data to this function  */
  curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, (void *) &chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt (curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

//fetch the http response code
  curl_easy_setopt (curl_handle, CURLOPT_HEADERFUNCTION, get_retcode_func);
  curl_easy_setopt (curl_handle, CURLOPT_HEADERDATA, &code);

  curl_easy_setopt (curl_handle, CURLOPT_ERRORBUFFER, err_buff);

  /* get it! */
  ret = curl_easy_perform (curl_handle);

  /* cleanup curl stuff */
  curl_easy_cleanup (curl_handle);

  /* if curl error occur,output the error messages */
  if (ret != 0)
    {
      printf ("[REASON-CURL]:%s\n", err_buff);
//发了一个信号，表明是curl_mem()出错退出了
      handler(1000);
    }
  
  /*insert http response code */
  printf ("http return code is %d\n", code);
  sprintf (sql_str, "update crawl_site set ret_code='%d' where id='%d'", code,
	   count);
  if (sql_query (sql_str))
    {
      //success
    }
  else
    {
      //failure
      printf ("insert %dth link's http response code[%d] error!\n", count,
	      code);
    }
  memset (sql_str, 0x00, 2048);

  /*fetch depth,prepare to insert when prm() */
  sprintf (sql_str, "select depth from crawl_site where id='%d'", count);
  //depth maybe 3 value:"NULL" "1" "failure"
  if ((depth = sql_fetch (sql_str))=='\0')
    {
    }
  else
    {
      if (memcmp (depth, cmp_abn, 8) == 0)
	{
	  printf ("depth's sql_fetch() error!\n");
	  exit (0);
	}
    }
  memset (sql_str, 0x00, 2048);
  printf ("now depth is %s.\n", depth);

  /*
   * Now, our chunk.memory points to a memory block that is chunk.size
   * bytes big and contains the remote file.
   *
   * Do something nice with it!
   *
   * You should be aware of the fact that at this point we might have an
   * allocated data block, and nothing has yet deallocated that data. So when
   * you're done with it, you should free() it as a nice application.
   */

//  printf ("%lu bytes retrieved\n", (long) chunk.size);
//  printf ("%s", chunk.memory);
  if (prm (chunk.memory, link, atoi(depth)) == 1)
    {
      //printf ("[DEBUG]:prm() execute!\n");
    }
  if (chunk.memory)
    free (chunk.memory);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup ();

  return 1;
}
