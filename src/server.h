
#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // close()
#include <sys/socket.h> // socket functions
#include <stdlib.h>     // exit()
#include <stdio.h>      // perror()
#include <string.h>     // memset()
#include <pthread.h>    // threads

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
  int n_clients;
  int map_size;
  pthread_mutex_t mutex;
} GameState;

typedef struct {
    GameState *game;
    int client_socket;
} ThreadArgs;

int createServerSocket();
void configureServerAddress(struct sockaddr_in *address);
void bindAndListen(int server_fd, struct sockaddr_in *address);
int *acceptClientConnection(int server_fd, struct sockaddr_in *address);
void *handleClient(void *arg);

void initSnake(Snake *snake, int socket, int start_x, int start_y);
void addClient(GameState *game, int socket);
void removeClient(GameState *game, int socket);
void checkFoodCollision(GameState *game);
void broadcastGameState(GameState *game);
void *gameLoop(void *arg);
int generateCords(GameState *game);

#endif
