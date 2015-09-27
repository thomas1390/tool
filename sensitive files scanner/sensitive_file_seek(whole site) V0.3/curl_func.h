#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

/* ******************************************************************** */
//结构体不需要申请内存，里面的指针需要申请内存
//http协议版本
//http respnsoe code
//http Content-length
//http Content-type
struct header
{
  float version;
  int code;
  long int length;
  char type[64];
};
//parse  the http response header
//说明*stream是位置的，libcurl里面的*stream以后都这样用
size_t
parse_return_header (void *ptr, size_t size, size_t nmemb, void *stream)
{
  int r;
  struct header *data = (struct header *) stream;

  if (strncmp (ptr, "HTTP/", 5) == 0)
    {
      r = sscanf (ptr, "HTTP/%f %d", &(data->version), &(data->code));
      if (r != 2)
	printf
	  ("parse_return_header() --sscanf http-response-code failure: %s\n",
	   strerror (errno));
    }

  if (strncmp (ptr, "Content-Length: ", 16) == 0)
    {
      r = sscanf (ptr, "Content-Length: %ld", &(data->length));
      if (r != 1)
	printf ("parse_return_header() --sscanf Content-Length failure: %s\n",
		strerror (errno));
    }

  if (strncmp (ptr, "Content-Type: ", 14) == 0)
    {
      r = sscanf (ptr, "Content-Type: %s", data->type);
      if (r != 1)
	printf ("parse_return_header() --sscanf Content-Type failure: %s\n",
		strerror (errno));
    }

  /*the file should less than 512k bytes
     if (data->length > 512000)
     {
     return 0;
     }
     else
     {
   */
  return size * nmemb;
  /*
     }
   */
}

/* ******************************************************************** */
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

int
curl_mem (char *url, long int port, int head_method, int cookie_need)
{
  CURL *curl_handle;
  char err_buff[CURL_ERROR_SIZE];
  CURLcode ret;
  struct header http_head;
  struct MemoryStruct chunk;

  chunk.memory = malloc (1);	/* will be grown as needed by the realloc above */
  chunk.size = 0;		/* no data at this point */

  curl_global_init (CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init ();

  /* specify URL to get */
  curl_easy_setopt (curl_handle, CURLOPT_URL, url);

  /* specify a port to get */
  curl_easy_setopt (curl_handle, CURLOPT_PORT, port);

  /* support gzip algorithm */
  curl_easy_setopt (curl_handle, CURLOPT_ENCODING, "");

  /*follow 302 response code
     curl_easy_setopt (curl_handle, CURLOPT_FOLLOWLOCATION, 1);
   */

  //set dns cache time-out && also need net start nscd
  curl_easy_setopt (curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, -1);
  //set CONNECTTIMEOUT
  curl_easy_setopt (curl_handle, CURLOPT_CONNECTTIMEOUT, 10);

  //support cookie authentication
  if (cookie_need)
    {
      curl_easy_setopt (curl_handle, CURLOPT_COOKIEFILE, "cookie.txt");
    }

  /* send all data to this function  */
  curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, (void *) &chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt (curl_handle, CURLOPT_USERAGENT,
		    "Opera/9.80 (Windows NT 5.0; U; en) Presto/2.7.62 Version/11.01");

//fetch the http response code
  curl_easy_setopt (curl_handle, CURLOPT_HEADERFUNCTION, parse_return_header);
  curl_easy_setopt (curl_handle, CURLOPT_HEADERDATA, (void *) &http_head);

//use HEAD method
  if (head_method)
    {
      curl_easy_setopt (curl_handle, CURLOPT_CUSTOMREQUEST, "HEAD");
    }

  curl_easy_setopt (curl_handle, CURLOPT_ERRORBUFFER, err_buff);

  /* get it! */
  ret = curl_easy_perform (curl_handle);

  /* cleanup curl stuff */
  curl_easy_cleanup (curl_handle);

  /* if curl error occur,output the error messages */
  if (ret != 0)
    {
      printf ("[curl_easy_perform failed!][URL]:%s[REASON]:%s\n", url,
	      err_buff);
    }

  if (http_head.code == 200 || http_head.code == 301 || http_head.code == 302)
    {
      if (strstr (http_head.type, "text/html") == 0)
	{
	  printf ("found a url: %s\n", url);
	}
    }

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
  if (chunk.memory)
    free (chunk.memory);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup ();
  return 1;
}
