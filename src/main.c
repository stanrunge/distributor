#include "p2p/p2p.h"
#include "web/server.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main() {
  if (start_web_server() != 0) {
    fprintf(stderr, "Failed to start web server\n");
    exit(EXIT_FAILURE);
  }

  if (start_p2p_server() != 0) {
    fprintf(stderr, "Failed to start P2P server\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    char command[100];
    //   get_command(command);
    //   run_command(command);
  }

  return 0;
}
