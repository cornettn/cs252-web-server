#include "http_messages.h"

#include <stdio.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int exists(char *filepath) {
  char *full_path = (char *) malloc(PATH_MAX);
  full_path = realpath(filepath, full_path);
  printf("Full path: %s\n", full_path);
  return access(full_path);
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

  char *url = request->request_uri;
  char *full_url = (char *) malloc(strlen(url) + strlen(ROOT));
  sprintf(full_url, "%s%s", ROOT, url);
  printf("url: %s\n", full_url);
  exists(full_url);

  return *resp;
}

