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


int is_directory(char *filepath) {
  char *full_path = get_realpath(filepath);
  struct stat buf = {0};
  if (stat(full_path, &buf) != 0) {

    /* Some type of error occured */

    return 0;
  }
  return S_ISDIR(buf.st_mode);
}

int exists(char *filepath) {
  char *full_path = get_realpath(filepath);
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



  if (is_directory(full_url)) {
    printf("%s is a directory\n", full_url);
    if (full_url[strlen(full_url) - 1] != FRONTSLASH) {

      /* Request will server the directory/index.html */

      full_url = (char *) realloc(full_url, strlen(full_url) +
                                            1 +
                                            strlen(INDEX));
      sprintf(full_url, "%s%c%s", full_url, FRONTSLASH, INDEX);
      if (!exists(full_url)) {
        printf("%s does not exist\n", full_url);
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
    printf("%s is not a dir\n", full_url);
  }

  if (!exists(full_url)) {
    // TODO Send a 404 response
    printf("%s does not exist\n", full_url);
    printf("404\n");
    return handle_default(request);
  }
  else {
    printf("%s does exist\n", full_url);
  }

  /* File exists */

  /* Not a directory and file exists */

  char *absolute_path = get_realpath(full_url);
  char *content_type = get_content_type(absolute_path);
  printf("%s\n", content_type);

  return *resp;
}

