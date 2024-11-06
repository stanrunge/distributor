#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct Node {
  char ip[INET_ADDRSTRLEN];
  int port;
  struct Node *next;
} Node;

typedef struct Message {
  char type[20];
  char payload[256];
} Message;

Node *head = NULL;

void create_socket() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in address;
  int opt = 1;

  // Creates a socket using IPv4 and TCP
  if (server_fd == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) < 0) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  // Set address to use IPv4
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Listening on port 8080\n");

  int new_socket;
  int addrlen = sizeof(address);

  new_socket =
      accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

  if (new_socket < 0) {
    perror("accept failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Connection accepted\n");
}

void bind_socket() { return; }

void setup_client() { printf("Setting up client...\n"); }
