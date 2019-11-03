#include "http_messages.h"

#include <stdio.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "misc.h"
#include "routes.h"

#define FRONTSLASH ('/')
#define INDEX ("index.html")

char *get_realpath(char *relative) {
  char *full_path = (char *) malloc(PATH_MAX);
  full_path = realpath(relative, full_path);
  printf("rel:  %s\nfull: %s\n", relative, full_path);
  return full_path;
}


int is_directory(char *full_path) {
  struct stat buf = {0};
  if (stat(full_path, &buf) != 0) {

    /* Some type of error occured */

    return 0;
  }
  return S_ISDIR(buf.st_mode);
}

int exists(char *full_path) {
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

  char *absolute_path = get_realpath(full_url);

  if (is_directory(absolute_path)) {
    printf("%s is a directory\n", absolute_path);
    if (absolute_path[strlen(absolute_path) - 1] != FRONTSLASH) {

      /* Request will server the directory/index.html */

      absolute_path = (char *) realloc(absolute_path, strlen(absolute_path) +
                                            1 +
                                            strlen(INDEX));
      sprintf(absolute_path, "%s%c%s", absolute_path, FRONTSLASH, INDEX);
      if (!exists(absolute_path)) {
        printf("%s does not exist\n", absolute_path);
        // TODO Send a 404 response
        printf("404\n");
        return handle_default(request);
      }
    }
    else {
      /* Browsable directories */
      printf("Browsable\n");
    }
  }
  else {
    printf("%s is not a dir\n", absolute_path);
  }

  if (!exists(full_url)) {
    // TODO Send a 404 response
    printf("%s does not exist\n", absolute_path);
    printf("404\n");
    return handle_default(request);
  }
  else {
    printf("%s does exist\n", absolute_path);
  }

  /* File exists */

  /* Not a directory and file exists */

  char *content_type = get_content_type(absolute_path);
  printf("%s\n", content_type);

  return *resp;
}

