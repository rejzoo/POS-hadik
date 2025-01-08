
#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <pthread.h>
#include "menu.h"

// TODO make util class ?
#define PORT 8090
#define MAX_BODY_LENGTH 99

typedef struct {
  int x, y;
} Position;

typedef struct {
  int map_size;
  int n_snakes;
  int n_bodies;
  Position *snakes_heads;
  Position *snakes_bodies;
  Position *foods;
} DataFromServer;

void startServer();
char getKey();
void drawGame(const DataFromServer *serverData);
void parseData(const char *data, DataFromServer *serverData);
void freeDataFromServer(DataFromServer *serverData);
void *receiveUpdates(void *arg);
void joinGame();
void handleChoice(int choice);

#endif
