#include "server.h"

#include <stdio.h>
#include <stdlib.h>

#include "http_messages.h"

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
 * Handle an incoming connection on the passed socket.
 */

void handle(socket_t *sock) {
  http_request request;

  // TODO: Replace this code and actually parse the HTTP request

  request.method = "GET";
  request.request_uri = "/";
  request.query = "";
  request.http_version = "HTTP/1.1";
  request.num_headers = 0;
  request.message_body = "";

  print_request(&request);


  http_response response = { 0 };

  // TODO: Add your code to create the correct HTTP response

  response.http_version = "HTTP/1.1";

  char *to_string = response_string(&response);
  printf("%s\n", to_string);
  socket_write_string(sock, to_string);

  free(to_string);
  to_string = NULL;

  close_socket(sock);
} /* handle() */
