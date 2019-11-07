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
#include "main.h"
#include "routes.h"

#define FRONTSLASH ('/')
#define INDEX ("index.html")


int get_file_size(char *path) {
  struct stat buf = {0};
  stat(path, &buf);
  return (int) buf.st_size;
}

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

int readable(char *full_path) {
  return (access(full_path, R_OK) == 0);
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
    mylog("Request is directory");
    if (absolute_path[strlen(absolute_path) - 1] != FRONTSLASH) {

      /* Request will server the directory/index.html */

      absolute_path = (char *) realloc(absolute_path, strlen(absolute_path) +
                                            1 +
                                            strlen(INDEX));
      sprintf(absolute_path, "%s%c%s", absolute_path, FRONTSLASH, INDEX);
      if (!exists(absolute_path)) {

        /* Send a 404 Response */

        return handle_default(request);
      }
    }
    else {
      /* Browsable directories */
      printf("Browsable\n");
    }
  }

  if (!exists(absolute_path)) {

    /* Return a 404 response */
    mylog("Request does not exist");
    return handle_default(request);
  }

  /* File exists */

  if (!readable(absolute_path)) {
    mylog("Request is forbidden");
    return handle_request(request, 403);
  }

  /* Not a directory and file exists */

  mylog("Valid Request");

  char *content_type = "type/plain;"; //get_content_type(absolute_path);

  char *semi_colon = strchr(content_type, ';');
  char *type = substring(content_type, 0, semi_colon - content_type);

  printf("Type: %s\n", type);

  header *type_head = (header *) malloc(sizeof(header));
  type_head->key = "Content-Type";
  type_head->value = strdup(type);
  free(type);
  type = NULL;

  add_header_to_response(resp, type_head);

  header *connection_head = (header *) malloc(sizeof(header));
  connection_head->key = "Connection";
  connection_head->value = "close";
  add_header_to_response(resp, connection_head);

  header *size = (header *) malloc(sizeof(header));
  size->key = "Content-Length";
  char *file_size = (char *) malloc(1024);
  sprintf(file_size, "%d", get_file_size(absolute_path));
  size->value = strdup(file_size);
  free(file_size);
  file_size = NULL;
  add_header_to_response(resp, size);

  resp->status_code = 200;
  resp->reason_phrase = (char *) status_reason(resp->status_code);

  FILE *fp = fopen(absolute_path, "r");
  fseek(fp, 0, SEEK_END);
  int length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *data = (char *) malloc(length + 1);
  fread(data, sizeof(char), length, fp);
  data[length] = '\0';
  resp->message_body = strdup(data);
  free(data);
  data = NULL;

  mylog("Response generated");

  return *resp;
}

