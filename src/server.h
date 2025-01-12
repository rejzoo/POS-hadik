
#ifndef SERVER_H
#define SERVER_H

#include "snake.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h> // sockaddr_in
#include <pthread.h>    // threads
#include <stdio.h>      // perror()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <sys/socket.h> // socket functions
#include <unistd.h>     // close()

typedef struct {
  Snake *snakes;
  Position *foods;
  int n_clients;
  int map_size;
  int max_clients;
  int *time;
  pthread_mutex_t mutex;
} GameState;

typedef struct {
  GameState *game;
  int client_socket;
} ThreadArgs;

int createServerSocket();
void configureServerAddress(struct sockaddr_in *address);
void bindAndListen(int server_fd, struct sockaddr_in *address);
int acceptClientConnection(int server_fd, struct sockaddr_in *address);
void *handleClient(void *arg);

void addClient(GameState *game, int socket);
void removeClient(GameState *game, int socket);
void checkFoodCollision(GameState *game);
void checkSnakeCollision(GameState *game);
void broadcastGameState(GameState *game);
void *gameLoop(void *arg);
int generateCords(GameState *game);
int outOfBounds(Snake *snake, GameState *game);
void initSnakeOnServer(Snake *snake);

#endif
