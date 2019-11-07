#include "main.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "server.h"
#include "socket.h"

#define DEFAULT_PORT (3001)
#define UNUSED(x) (void)(x)

int g_debug = 0;

/*
 * Print the usage for the program. Takes the name of the program as an
 * argument.
 */

void print_usage(char *name) {
    printf("USAGE: %s [-f|-t|-pNUM_THREADS] [-h] PORT_NO\n", name);
} /* print_usage() */

void mylog(char *msg) {
  return;
  // dprintf(g_debug, "%s\n", msg);
}

void sig_child_handler(int sig, siginfo_t *info, void *context) {
  UNUSED(sig);
  UNUSED(context);

  waitpid(info->si_pid, 0, WNOHANG);
}


/*
 * Runs the webserver.
 */

int main(int argc, char **argv) {

  struct sigaction sig_act = {0};
  sig_act.sa_sigaction = sig_child_handler;
  sigemptyset(&sig_act.sa_mask);
  sig_act.sa_flags = SA_NOCLDWAIT | SA_RESTART | SA_SIGINFO;
  int err = sigaction(SIGCHLD, &sig_act, NULL);

  if (err) {
    perror("sigaction");
    exit(-1);
  }


  g_debug = open("logs", O_CREAT | O_RDWR, 0666);

  mylog("fd is open");

  // default to linear

  concurrency_mode mode = E_NO_CONCURRENCY;

  int port_no = 0;
  int num_threads = 0;

  int c = 0;
  while ((c = getopt(argc, argv, "hftp:")) != -1) {
    switch (c) {
    case 'h':
      print_usage(argv[0]);
      return 0;
    case 'f':
    case 't':
    case 'p':
      if (mode != E_NO_CONCURRENCY) {
        fputs("Multiple concurrency modes specified\n", stdout);
        print_usage(argv[0]);
        return -1;
      }
      mode = (concurrency_mode) c;
      if (mode == E_THREAD_POOL) {
        num_threads = atoi(optarg);
      }
      break;
    case '?':
      if (isprint(optopt)) {
        fprintf(stderr, "Unknown option: -%c\n", optopt);
      }
      else {
        fprintf(stderr, "Unknown option\n");
      }
      // Fall through
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  if (optind > argc) {
    fprintf(stderr, "Extra arguments were specified");
    print_usage(argv[0]);
    return 1;
  }
  else if (optind == argc) {
    // use default port

    port_no = DEFAULT_PORT;
  }
  else {
    port_no = atoi(argv[optind]);
  }

  printf("%d %d %d\n", mode, port_no, num_threads);

  mylog("main.c: Create socket acceptor");

  acceptor *acceptor = create_socket_acceptor(port_no);

  if (acceptor == NULL) {
	  return 1;
  }

  switch (mode) {
  case E_FORK_PER_REQUEST:
    mylog("main.c: run forking sever");
    run_forking_server(acceptor);
    break;
  case E_THREAD_PER_REQUEST:
    mylog("main.c: run threaded sever");
    run_threaded_server(acceptor);
    break;
  case E_THREAD_POOL:
    mylog("main.c: run pooled sever");
    run_thread_pool_server(acceptor, num_threads);
    break;
  default:
    mylog("main.c: run linear sever");
    run_linear_server(acceptor);
    break;
  }

  mylog("Close main socket acceptor");

  close_socket_acceptor(acceptor);

  return 0;
} /* main() */


