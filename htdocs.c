#include "http_messages.h"

#include <stdio.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int is_directory(char *filepath) {
  char *full_path = (char *) malloc(PATH_MAX);
  full_path = realpath(filepath, full_path);
  struct stat buf = {0};
  if (stat(full_path, &buf) != 0) {

    /* Some type of error occured */

    return 0;
  }
  return S_ISDIR(buf.st_mode);
}

int exists(char *filepath) {
  char *full_path = (char *) malloc(PATH_MAX);
  full_path = realpath(filepath, full_path);
  printf("Full path: %s\n", full_path);
  return (access(full_path, F_OK) == 0);
}

/*
 * You should implement this function and use it in server.c
 */

http_response handle_htdocs(const http_request *request) {
  printf("yoyo\n");

  http_response *resp = (http_response *) malloc(sizeof(http_response));
  resp->http_version = request->http_version;
  resp->num_headers = 0;
  resp->headers = NULL;

    // char *referer = get_header_value(request, REFERER);
    // printf("%s\n", referer);

  // TODO: Get the request URL, verify the file exists, and serve it

  /* Get the requested URL */

  char *url = request->request_uri;
  char *full_url = (char *) malloc(strlen(url) + strlen(ROOT));
  sprintf(full_url, "%s%s", ROOT, url);


  if (!exists(full_url)) {
    // TODO Send a 404 response
  }

  /* File exists */

  if (is_directory(full_url)) {
    printf("Dir\n");
  }

  return *resp;
}

