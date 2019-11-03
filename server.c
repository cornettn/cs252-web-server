#include "server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_messages.h"
#include "routes.h"

#define BUF_SIZE (1024)
#define TRUE (1)
#define FALSE (0)
#define SUCCESS (1)
#define PARSE_ERROR (-1)
#define CRLF ("\r\n")
#define AUTH_HEADER ("Authorization")

int parse_request(http_request *, socket_t *);
char *substring(char *, int, int);

char g_user_pass[MAX_USR_PWD_LEN];

/*
 * Return a string in a format <user>:<password>
 * either from auth.txt or from your implememtation.
 */

char *return_user_pwd_string(void) {
  // Read from ./auth.txt. Don't change this. We will use it for testing
  FILE *fp = NULL;
  char *line = (char *)malloc(sizeof(char) * MAX_USR_PWD_LEN);
  size_t len = 0;

  fp = fopen("./auth.txt", "r");
  if (fp == NULL) {
    perror("couldn't read auth.txt");
    exit(-1);
  }

  if (getline(&line, &len, fp) == -1) {
    perror("couldn't read auth.txt");
    exit(-1);
  }

  sprintf(g_user_pass, "%s", line);

  free(line);

  return g_user_pass;
} /* return_user_pwd_string() */

/*
 * Accept connections one at a time and handle them.
 */

void run_linear_server(acceptor *acceptor) {
  while (1) {
    socket_t *sock = accept_connection(acceptor);
    handle(sock);
  }
} /* run_linear_server() */

/*
 * Accept connections, creating a different child process to handle each one.
 */

void run_forking_server(acceptor *acceptor) {
  // TODO: Add your code to accept and handle connections in child processes
} /* run_forking_server() */

/*
 * Accept connections, creating a new thread to handle each one.
 */

void run_threaded_server(acceptor *acceptor) {
  // TODO: Add your code to accept and handle connections in new threads
} /* run_threaded_server() */

/*
 * Accept connections, drawing from a thread pool with num_threads to handle the
 * connections.
 */

void run_thread_pool_server(acceptor *acceptor, int num_threads) {
  // TODO: Add your code to accept and handle connections in threads from a
  // thread pool
} /* run_thread_pool_server() */




/*
 * This fuction is used to parse an individual header.
 */

header *parse_header(char *head) {

  char *space = strchr(head, ' ');
  if (space == NULL) {
    //printf("Invalid Header\n");
    return NULL;
  }

  header *headr = (header *) malloc(sizeof(header));

  headr->key = substring(head, 0, space - head - 1);
  space++;
  headr->value = substring(space, 0, strlen(space));

  //printf("Key: {%s}\n", headr->key);
  //printf("Val: {%s}\n", headr->value);

  return headr;

} /* parse_header() */


int parse_all_headers(http_request *request, socket_t *sock, char *buf) {

  /* This is the location of the next CRLF in the headers */

  char *crlf = strstr(buf, CRLF);

  /* This is the string containing all the un-parsed headers */

  char *rest_headers = crlf + strlen(CRLF);

  /* Find the location of the next CRLF */

  crlf = strstr(rest_headers, CRLF);

  while (crlf != NULL) {
    /* Extract the header */

    char *headr = substring(rest_headers, 0, crlf - rest_headers);

    header *head = parse_header(headr);
    free(headr);
    headr = NULL;
    if (head == NULL) {
      return PARSE_ERROR;
    }

    /* Update the request */

    request->num_headers++;
    request->headers = realloc(request->headers,
      request->num_headers * sizeof(header));
    request->headers[request->num_headers - 1] = *head;

    /* Update rest_headers and crlf */

    rest_headers = crlf + strlen(CRLF);
    crlf = strstr(rest_headers, CRLF);

    /* Break the loop if the last crlf and next crlf are next to each other */

    if (rest_headers ==  crlf) {
      break;
    }
  }

  return SUCCESS;
}

/*
 * This function is used to parse an incoming http request.
 */

int parse_request(http_request *request, socket_t *sock) {
  /* Ensure that the request is filled with default values */
  request->method = "method";
  request->request_uri = "uri";
  request->http_version = "http_version";
  request->num_headers = 0;;
  request->query = "query";
  request->message_body = "message body";

  /* Buffer to hold the contents of the socket */

  char *buf = (char *) malloc(BUF_SIZE);

  /* Read the contents of sock into BUF.
   * Try reading BUF_SIZE characters */

  int check = socket_read(sock, buf, BUF_SIZE);
  if (check == -1) {

    /* Error */

    fprintf(stderr, "error\n");
  }
  else {
    printf("Contents of Socket:\n----------------\n%s\n----------------\n", buf);
  }


  /* Parse the method */
  /* Pointer to the first space in buffer */

  char *space = strchr(buf, ' ');
  char *method = NULL;
  if (space != NULL) {
    method = substring(buf, 0, space - buf);
    request->method = strdup(method);
    free(method);
    method = NULL;
  }
  else {
    return PARSE_ERROR;
  }

  /* Parse the request uri */
  /* Pointer to the first character of the request uri */

  char *request_uri = space + 1;
  space = strchr(request_uri, ' ');
  char *uri = NULL;
  if (space != NULL) {
    uri = substring(request_uri, 0, space - request_uri);
    request->request_uri = strdup(uri);
    free(uri);
    uri = NULL;
  }
  else {
    return PARSE_ERROR;
  }

  /* Parse the http version */
  /* Pointer to the first character of the version */

  char *version = space + 1;
  space = strstr(version, "\r\n");
  char *http_version = NULL;
  if (space != NULL) {
    http_version = substring(version, 0, space - version);
    request->http_version = strdup(http_version);
    free(http_version);
    http_version = NULL;
  }
  else {
    return PARSE_ERROR;
  }

  return parse_all_headers(request, sock, buf);
} /* parse_request() */

/*
 * This function is used to get the value of the header key.
 */

char *get_header_value(http_request *request, char *key) {
  for (int i = 0; i < request->num_headers; i++) {
    if (!strcmp(request->headers[i].key, key)) {
      return request->headers[i].value;
    }
  }
  return NULL;
} /* get_header_value() */


/*
 * This function is used to check if a request is authorized.
 */

int is_authorized(http_response *resp, http_request *req) {
  char *auth = get_header_value(req, AUTH_HEADER);

  /* Ensure that there is a Authorization header */

  if (auth != NULL) {
    char *space = strchr(auth, ' ');
    space++;
    char *base64 = substring(space, 0, strlen(space));
    base64 = strcat(base64, "\n");


    printf("Request: {%s}\n", base64);
    printf("Require: {%s}\n", g_user_pass);

    /* Ensure that the username and password are correct */

    if (!strcmp(base64, g_user_pass)) {
      /* Authorized */
      return TRUE;
    }
  }

  /* Not Authorized */

  header *head = (header *) malloc(sizeof(header));
  head->key = strdup("WWW-Authenticate");
  head->value = strdup("Basic realm=\"hardcode\"");
  add_header_to_response(resp, head);
  resp->status_code = 401;
  resp->reason_phrase = strdup(status_reason(resp->status_code));
  //return FALSE;

  // TODO Change back to FALSE after testing

  return TRUE;
} /* is_authorized() */

/*
 * Handle an incoming connection on the passed socket.
 */

void handle(socket_t *sock) {
  return_user_pwd_string();

  http_request request = {0};

  /* Parse the Request */

  int value = parse_request(&request, sock);
  if (value == PARSE_ERROR) {
    fprintf(stderr, "Error parsing the request\n");
  }
  print_request(&request);

  /* Write the response */

  http_response *response = (http_response *) malloc(sizeof(http_response));
  response->http_version = request.http_version;
  response->num_headers = 0;
  response->headers = NULL;

  if(is_authorized(response, &request)) {
    // TODO Handle a request that is authorized
    printf("YAYYYY Authorized!\n");
    http_response resp = handle_htdocs(&request);
    response = &resp;
  }


  // PRIORITY 2
  // TODO: Add your code to create the correct HTTP response

  //response = handle_htdocs(&request);


  char *to_string = response_string(response);
  printf("%s\n", to_string);
  socket_write_string(sock, to_string);

  free(to_string);
  to_string = NULL;

  close_socket(sock);
} /* handle() */


/*http_response handle_htdocs(const http_request *request) {
  http_response resp = {0};
  resp.http_version = request->http_version;
  resp.status_code = 200;
  resp.reason_phrase = status_reason(resp.status_code);
}*/

