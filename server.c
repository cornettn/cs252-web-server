#include "server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include "http_messages.h"
#include "routes.h"
#include "main.h"

#define BUF_SIZE (1024)
#define TRUE (1)
#define FALSE (0)
#define SUCCESS (1)
#define PARSE_ERROR (-1)
#define CRLF ("\r\n")
#define AUTH_HEADER ("Authorization")

#define SIZEOF(x) (sizeof(x) / sizeof(char *))

int parse_request(http_request *, socket_t *);
char *substring(char *, int, int);

char g_user_pass[MAX_USR_PWD_LEN];

char *accepted_methods[] = {"GET"};
char *accepted_http_versions[] = {"HTTP/1.1", "HTTP/1.0"};

/*
 * Return a string in a format <user>:<password>
 * either from auth.txt or from your implememtation.
 */

char *return_user_pwd_string(void) {

  // Read from ./auth.txt. Don't change this. We will use it for testing

  FILE *fp = NULL;
  char *line = NULL;
  size_t len = 0;

  fp = fopen("./auth.txt", "r");

  if (fp == NULL) {
    perror("couldn't read auth.txt");
    exit(-1);
  }

  if (getline(&line, &len, fp) == -1) {
    perror("couldn't read auth.txt");
    free(line);
    line = NULL;
    exit(-1);
  }

  sprintf(g_user_pass, "%s", line);

  free(line);
  line = NULL;
  fclose(fp);

  return g_user_pass;
} /* return_user_pwd_string() */

/*
 * Accept connections one at a time and handle them.
 */

void run_linear_server(acceptor *acceptor) {
  return_user_pwd_string();
  while (1) {
    socket_t *sock = accept_connection(acceptor);

    mylog("server.c: handle socket");

    handle(sock);
  }
} /* run_linear_server() */

/*
 * Accept connections, creating a different child process to handle each one.
 */

void run_forking_server(acceptor *acceptor) {
  // TODO: Add your code to accept and handle connections in child processes
  return_user_pwd_string();
  while (1) {
    socket_t *sock = accept_connection(acceptor);
    //mylog("Connection accepted");
    int pid = fork();
    if (pid == -1) {
      perror("Forking error in forking_server");
      exit(-1);
    }
    if (pid == 0) {

      /* Child Process */

      mylog("Start Child Process");
      handle(sock);
      mylog("Exit child process");

      exit(0);
    }

    /* Parent should keep looping */

    close_socket(sock);

  }

} /* run_forking_server() */

/*
 * Accept connections, creating a new thread to handle each one.
 */

void run_threaded_server(acceptor *acceptor) {
  // TODO: Add your code to accept and handle connections in new threads
  return_user_pwd_string();
  while(1) {
    socket_t *sock = accept_connection(acceptor);
    pthread_t thrd = {0};

    pthread_attr_t attributes = {0};
    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes,
        PTHREAD_CREATE_DETACHED);
    pthread_create(&thrd, &attributes,(void * (*)(void *)) handle, (void *) sock);
  }

} /* run_threaded_server() */

void *thread_pool_loop(acceptor *acceptor) {
  while(1) {
    socket_t *sock = accept_connection(acceptor);
    handle(sock);
  }
}


/*
 * Accept connections, drawing from a thread pool with num_threads to handle the
 * connections.
 */

void run_thread_pool_server(acceptor *acceptor, int num_threads) {
  // TODO: Add your code to accept and handle connections in threads from a
  // thread pool
  return_user_pwd_string();

  pthread_t *threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
  pthread_attr_t attributes = {0};
  pthread_attr_init(&attributes);
  pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

  for (int i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], &attributes,
        (void *(*)(void *)) thread_pool_loop,
        (void *) acceptor);
  }

  thread_pool_loop(acceptor);

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

  mylog("Read the sock");

  int check = socket_read(sock, buf, BUF_SIZE);
  if (check == -1) {

    /* Error */

    fprintf(stderr, "error\n");
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


char *decode(char *str) {

  printf("decode: {%s}\n", str);

  FILE *fp = NULL;
  char *command = (char *) malloc(BUF_SIZE);
  char *buf = (char *) malloc(BUF_SIZE);

  sprintf(command, "echo \"%s\" > test_auth", str);
  system(command);

  fp = popen("/usr/bin/base64 -d test_auth", "r");

  char *decoded = (char *) malloc(BUF_SIZE);
  decoded[0] = '\0';

  while(fgets(buf, sizeof(buf), fp) != NULL) {
    if (decoded[0] != '\0') {
      decoded = strcat(decoded, buf);
    }
    else {
      decoded = strdup(buf);
    }
  }
  free(buf);
  buf = NULL;
  fclose(fp);
  fp = NULL;
  return decoded;
}


int accepted_http_version(char *version) {
  mylog("Check to see if valid http version");
  for (int i = 0; i < SIZEOF(accepted_http_versions); i++) {
    if (!strcmp(version, accepted_http_versions[i])) {
      mylog("Valid http version");
      return 1;
    }
  }
  mylog("Not valid http version");
  return 0;
}


int accepted_method(char *method) {
  mylog("Check to see if accepted method");
  for (int i = 0; i < SIZEOF(accepted_methods); i++) {
    if (!strcmp(method, accepted_methods[i])) {
      mylog("Accepted method");
      return 1;
    }
  }
  mylog("Not accepted method");
  return 0;
}


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
    char *decoded = decode(base64);
    free(base64);
    base64 = NULL;

    char *command = malloc(BUF_SIZE);
    sprintf(command, "rm -f passed_auth");
    system(command);
    free(command);
    command = NULL;

    decoded = strcat(decoded, "\n");

    /* Ensure that the username and password are correct */

    printf("Decoded: {%s}\n"
           "Password:{%s}\n", decoded, g_user_pass);

    if (!strcmp(decoded, g_user_pass)) {

      /* Authorized */
      mylog("authorized");
      free(decoded);
      decoded = NULL;
      return TRUE;
    }
  }

  /* Not Authorized */

  mylog("Not Authorized");

  header *head = (header *) malloc(sizeof(header));
  head->key = strdup("WWW-Authenticate");
  head->value = strdup("Basic realm=\"noah-cornett-realm\"");
  add_header_to_response(resp, head);
  resp->status_code = 401;
  resp->reason_phrase = strdup(status_reason(resp->status_code));
  resp->message_body = "Unauthorized";
  return FALSE;
} /* is_authorized() */

/*
 * Handle an incoming connection on the passed socket.
 */

void handle(socket_t *sock) {
  //return_user_pwd_string();
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

  if (value == PARSE_ERROR) {
    http_response resp = handle_request(&request, 400);
    response = &resp;
  }
  else if (!accepted_method(request.method)) {
    http_response resp = handle_request(&request, 405);
    response = &resp;
  }
  else if (!accepted_http_version(request.http_version)) {
    http_response resp = handle_request(&request, 505);
    response = &resp;
  }
  else if(is_authorized(response, &request)) {
    http_response resp = handle_htdocs(&request);
    response = &resp;
  }

  // PRIORITY 2
  // TODO: Add your code to create the correct HTTP response


  printf("Generating response\n");

  char *to_string = response_string(response);

  printf("aha {%s}\n", to_string);

  int len = get_response_length(response);

 // socket_write_string(sock, to_string);
  socket_write(sock, to_string, len);

  free(to_string);
  to_string = NULL;

  close_socket(sock);
} /* handle() */

