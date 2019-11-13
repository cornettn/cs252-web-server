#include "tls.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>



/*
 * Close and free a TLS socket created by accept_tls_connection(). Return 0 on
 * success. You should use the polymorphic version of this function, which is
 * close_socket() in socket.c.
 */

int close_tls_socket(tls_socket *socket) {
  // TODO: Add your code to close the socket
  printf("Closing TLS socket fd %d\n", socket->socket_fd);



  int status = close(socket->socket_fd);
  free(socket);
  return status;
} /* close_tls_socket() */

/*
 * Read a buffer of length buf_len from the TLS socket. Return the length of the
 * message on successful completion.
 * You should use the polymorphic version of this function, which is socket_read()
 * in socket.c
 */

int tls_read(tls_socket *socket, char *buf, size_t buf_len) {
  // TODO: Add your code to read from the socket
  if (buf == NULL){
    return -1;
  }

  buf[0] = '\0';

  return 0;
} /* tls_read() */

/*
 * Write a buffer of length buf_len to the TLS socket. Return 0 on success. You
 * should use the polymorphic version of this function, which is socket_write()
 * in socket.c
 */

int tls_write(tls_socket *socket, char *buf, size_t buf_len) {
  if (buf == NULL) {
    return -1;
  }

  // TODO: Add your code to write to the socket

  return 0;
} /* tls_write() */


void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context() {
  const SSL_METHOD *method = NULL;
  SSL_CTX *ctx = NULL;

  /* Create a new context with the specified method */

  method = TLS_server_method();
  ctx = SSL_CTX_new(method);
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  return ctx;
}

void configure_context(SSL_CTX *ctx) {
  SSL_CTX_set_ecdh_auto(ctx, 1);

  /* Load the certificate into context */

  if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  /* Load the private key into context */

  if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem" SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

void create_ssl_ctx(SSL_CTX *ctx) {
  init_openssl();
  ctx = create_context();
  configure_context(ctx);
}

/*
 * Create a new TLS socket acceptor, listening on the given port. Return NULL on
 * error. You should ues the polymorphic version of this function, which is
 * create_socket_acceptor() in socket.c.
 */

tls_acceptor *create_tls_acceptor(int port) {
  tls_acceptor *acceptor = malloc(sizeof(tls_acceptor));

  /* Set the IP address and port for this server */

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* Allocate a socket */

  int sock = 0;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Creating master socket");
    exit(EXIT_FAILURE);
  }

  /* Bind the socket to the IP address and port */

  if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("Unable to listen to master socket");
    exit(EXIT_FAILURE);
  }

  /* Put the socket into listening mode */

  if (listen(sock, 1) < 0) {
    perror("Unable to listen to master sock");
    exit(EXIT_FAILURE);
  }

  acceptor->master_socket = sock;
  acceptor->addr = addr;
  SSL_CTX *ssl_ctx = NULL;
  create_ssl_ctx(ssl_ctx);
  acceptor->ssl_ctx = ssl_ctx;

  return acceptor;
} /* create_tls_acceptor() */

/*
 * Accept a new connection from the TLS socket acceptor. Return NULL on error,
 * and the new TLS socket otherwise. You should use the polymorphic version of
 * this function, which is accept_connection() in socket.c.
 */

tls_socket *accept_tls_connection(tls_acceptor *acceptor) {
  // TODO: Add your code to create the new socket

  struct sockaddr_in addr = {0};
  socklen_t len = sizeof(addr);
  int client = accept(acceptor->master_socket,
      (struct sockaddr *) &addr, &len);
  if (client < 0) {
    perror("Unable to accept");
    return NULL;
  }

  SSL *ssl = SSL_new(acceptor->ssl_ctx);
  SSL_set_fd(ssl, acceptor->socket_fd);

  if (SSL_accept(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    return NULL;
  }

  tls_socket *sock = malloc(sizeof(tls_socket));
  sock->socket_fd = client;
  sock->addr = addr;
  sock->ssl = ssl;
  return sock;

} /* accept_tls_connection() */

/*
 * Close and free the passed TLS socket acceptor. Return 0 on success. You
 * should use the polymorphic version of this function, which is
 * close_socket_acceptor() in socket.c.
 */

int close_tls_acceptor(tls_acceptor *acceptor) {
  // TODO: Add your code to close the master socket

  free(acceptor);
  return -1;
} /* close_tls_acceptor() */
