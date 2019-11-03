#include "http_messages.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

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

    //char *URL = request->request_uri;


  return *resp;
}

