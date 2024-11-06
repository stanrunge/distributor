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

  //  init_socket();

  //  show_intro();

  while (1) {
    char command[100];
    //   get_command(command);
    //   run_command(command);
  }

  return 0;
}
