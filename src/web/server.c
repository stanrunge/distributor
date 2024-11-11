#include <arpa/inet.h>
#include <netinet/in.h> // for sockaddr_in
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <sys/stat.h> // for file statistics
#include <sys/types.h>
#include <unistd.h> // for close()

#define PORT 8080
#define BUFFER_SIZE 4096

static pthread_t server_thread;
static int server_running = 0;
static int server_fd = -1;

static void *web_server_thread(void *arg);
static char *read_file(const char *filepath, long *out_size);
static char *construct_http_response(const char *content, long content_length,
                                     const char *status);
static int send_http_response(int client_socket, const char *filepath);

int start_web_server() {
  if (server_running) {
    fprintf(stderr, "Server is already running\n");
    return -1;
  }

  server_running = 1;
  if (pthread_create(&server_thread, NULL, web_server_thread, NULL) != 0) {
    perror("Failed to create server thread");
    server_running = 0;
    return -1;
  }

  pthread_detach(server_thread);
  printf("Web Server started\n");
  return 0;
}

char *read_file(const char *filepath, long *out_size) {
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    perror("Failed to open file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  char *buffer = malloc(size + 1);
  if (!buffer) {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  size_t read = fread(buffer, 1, size, file);
  if (read != size) {
    perror("Failed to read file");
    fclose(file);
    free(buffer);
    return NULL;
  }

  buffer[size] = '\0';

  if (out_size) {
    *out_size = size;
  }

  fclose(file);
  return buffer;
}

char *construct_http_response(const char *content, long content_length,
                              const char *status) {
  const char *header_template = "HTTP/1.1 %s \r\n"
                                "Content-Type: text/html\r\n"
                                "Content-Length: %ld\r\n"
                                "Connection: close\r\n"
                                "\r\n";

  size_t header_size = strlen(header_template) + strlen(status) + 20;
  char *header = malloc(header_size);
  if (!header) {
    perror("Failed to allocate memory");
    return NULL;
  }

  sprintf(header, header_template, status, content_length);

  size_t total_size = strlen(header) + content_length + 1;
  char *response = malloc(total_size);
  if (!response) {
    perror("Failed to allocate memory");
    free(header);
    return NULL;
  }

  strcpy(response, header);
  strcat(response, content);

  free(header);
  return response;
}

int send_http_response(int client_socket, const char *filepath) {
  long file_size;
  char *file_content = read_file(filepath, &file_size);
  if (!file_content) {
    const char *not_found = "<!DOCTYPE html>"
                            "<html>"
                            "<head><title>404 Not Found</title></head>"
                            "<body><h1>404 Not Found</h1></body>";

    char *response =
        construct_http_response(not_found, strlen(not_found), "404 Not Found");
    if (response) {

      send(client_socket, response, strlen(response), 0);
      free(response);
    }
    return -1;
  }

  char *response = construct_http_response(file_content, file_size, "200 OK");
  if (response) {
    send(client_socket, response, strlen(response), 0);
    free(response);
  }

  return 0;
}

void *handle_client(void *arg) {
  int client_socket = *(int *)arg;
  free(arg);

  char buffer[BUFFER_SIZE];
  int read_bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
  if (read_bytes <= 0) {
    perror("Failed to read from socket");
    close(client_socket);
    pthread_exit(NULL);
  }

  buffer[read_bytes] = '\0';
  printf("Received request:\n%s\n", buffer);

  send_http_response(client_socket, "src/web/index.html");

  close(client_socket);
  pthread_exit(NULL);
}

static void *web_server_thread(void *arg) {
  int new_socket;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    pthread_exit(NULL);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    close(server_fd);
    pthread_exit(NULL);
  }

#ifdef SO_REUSEPORT
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    close(server_fd);
    pthread_exit(NULL);
  }
#endif

  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    pthread_exit(NULL);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen failed");
    close(server_fd);
    pthread_exit(NULL);
  }

  printf("Listening on port 8080\n");

  while (1) {
    new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0) {
      perror("Accept failed");
    }

    printf("Accepted connection from %s:%d\n", inet_ntoa(address.sin_addr),
           ntohs(address.sin_port));

    int *client_socket = malloc(sizeof(int));
    if (!client_socket) {
      perror("Failed to allocate memory for client socket");
      close(new_socket);
      continue;
    }

    *client_socket = new_socket;

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, handle_client, client_socket) !=
        0) {
      perror("Failed to create client thread");
      free(client_socket);
      close(new_socket);
      continue;
    }

    pthread_detach(client_thread);
  }

  close(server_fd);
  pthread_exit(NULL);
}
