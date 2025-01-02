
#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket functions
#include <stdlib.h>     // For exit()
#include <stdio.h>      // For perror()
#include <string.h>     // For memset()
#include <pthread.h>    // Threads

#define PORT 8090
#define MAX_CLIENTS 4

typedef struct {
  int socket;
  int x, y;
  int dx, dy;
  int alive;
} Snake;

typedef struct {
  int x, y;
} Food;

typedef struct {
  Snake snakes[MAX_CLIENTS];
  Food food[MAX_CLIENTS];
  int clients;
  pthread_mutex_t mutex;
} GameState;

int createServerSocket();
void configureServerAddress(struct sockaddr_in *address);
void bindAndListen(int server_fd, struct sockaddr_in *address);
int *acceptClientConnection(int server_fd, struct sockaddr_in *address);
void *handleClient(void *arg);

#endif
