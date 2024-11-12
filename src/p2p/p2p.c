#include <arpa/inet.h>
#include <netinet/in.h> // for sockaddr_in
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/socket.h>
#include <sys/stat.h> // for file statistics
#include <sys/types.h>
#include <unistd.h> // for close()

static int server_running;

static pthread_t server_thread;
static void *p2p_server_thread();

typedef struct {
  char ip[INET_ADDRSTRLEN];
  int port;
} Peer;

typedef struct {
  Peer peers[100];
  int count;
  pthread_mutex_t lock;
} PeerList;

int start_p2p_server() {
  if (server_running) {
    fprintf(stderr, "Server is already running\n");
    return -1;
  }
  server_running = 1;
  if (pthread_create(&server_thread, NULL, p2p_server_thread, NULL) != 0) {
    perror("Failed to create server thread");
    server_running = 0;
    return -1;
  }
  pthread_detach(server_thread);
  printf("P2P Server started\n");
  return 0;
}

void *p2p_server_thread() {
  int socket_fd;
  struct sockaddr_in local_addr;
  struct ip_mreq mreq;
  char buffer[2048];

  if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    pthread_exit(NULL);
  }

  u_int yes = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    perror("setsockopt failed");
    close(socket_fd);
    pthread_exit(NULL);
  }

  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(9000);
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
    perror("bind failed");
    close(socket_fd);
    pthread_exit(NULL);
  }

  mreq.imr_multiaddr.s_addr = inet_addr("239.0.0.1");
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
                 sizeof(mreq)) < 0) {
    perror("setsockopt failed");
    close(socket_fd);
    pthread_exit(NULL);
  }

  while (1) {
    int nbytes = recv(socket_fd, buffer, sizeof(buffer), 0);
    if (nbytes < 0) {
      perror("recv failed");
      close(socket_fd);
      pthread_exit(NULL);
    }

    printf("Received multicast message: %s\n", buffer);
  }

  if (setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq,
                 sizeof(mreq)) < 0) {
    perror("setsockopt failed");
    close(socket_fd);
    pthread_exit(NULL);
  }

  close(socket_fd);
  pthread_exit(NULL);
}

void *multicast_sender(void *arg) {
  int socket_fd;
  struct sockaddr_in multicast_address;
  char message[2048];
  char own_ip[INET_ADDRSTRLEN];
  int own_port = 9000;

  if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    pthread_exit(NULL);
  }

  unsigned char ttl = 1;
  if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) <
      0) {
    perror("setsockopt failed");
    close(socket_fd);
    pthread_exit(NULL);
  }

  memset(&multicast_address, 0, sizeof(multicast_address));
  multicast_address.sin_family = AF_INET;
  multicast_address.sin_addr.s_addr = inet_addr("239.0.0.1");
  multicast_address.sin_port = htons(9000);

  struct sockaddr_in own_address;
  socklen_t own_address_len = sizeof(own_address);
  int temp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (temp_socket == -1) {
    perror("socket creation failed");
    close(socket_fd);
    pthread_exit(NULL);
  }

  if (connect(temp_socket, (struct sockaddr *)&multicast_address,
              sizeof(multicast_address)) < 0) {
    perror("connect failed");
    close(temp_socket);
    close(socket_fd);
    pthread_exit(NULL);
  }

  if (getsockname(temp_socket, (struct sockaddr *)&own_address,
                  &own_address_len) < 0) {
    perror("getsockname failed");
    close(temp_socket);
    close(socket_fd);
    pthread_exit(NULL);
  }

  inet_ntop(AF_INET, &own_address.sin_addr, own_ip, INET_ADDRSTRLEN);
  close(temp_socket);

  snprintf(message, sizeof(message), "%s:%d", own_ip, own_port);

  while (1) {
    if (sendto(socket_fd, message, strlen(message), 0,
               (struct sockaddr *)&multicast_address,
               sizeof(multicast_address)) < 0) {
      perror("sendto failed");
    } else {
      printf("Sent multicast message: %s\n", message);
    }

    sleep(5);
  }

  close(socket_fd);
  return NULL;
}
