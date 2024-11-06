#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show_intro() {
  printf("-----------------------------------------\n\n");
  printf(" THE DISTRIBUTOR (Written by Stan Runge)\n\n");
  printf("-----------------------------------------\n\n");
}

char *get_command(char *command) {
  printf("Available commands:\n\n");

  printf("add - Add a new node\n");
  printf("kill - Kill a random node\n");
  printf("list - List all nodes\n");
  // printf("exit - Exit the program\n");
  printf("job - Start a new job\n");
  printf("broadcast - Broadcast a message to all nodes\n");

  printf("\n");
  printf("Enter a command: ");

  scanf("%s", command);

  return command;
}
