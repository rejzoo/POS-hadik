#ifndef CLIENT_H
#define CLIENT_H

#include "menu.h"
#include "utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
  int map_size;
  int n_snakes;
  int n_bodies;
  int time;
  Position *snakes_heads;
  Position *snakes_bodies;
  Position *foods;
} DataFromServer;

typedef struct Client Client;

void Client_init(Client *client);
void Client_startServer(char *map_size, char *max_clients, char *game_mode,
                        char *game_time);
void Client_joinGame(Client *client);
void Client_handleChoice(Client *client, int choice, char *map_size,
                         char *max_clients, char *game_mode, char *game_time);
void Client_drawGame(const DataFromServer *serverData, Client *client);
void Client_parseData(const char *data1, const char *data2,
                      DataFromServer *serverData, Client *client);
void Client_freeDataFromServer(DataFromServer *serverData);
void *Client_receiveUpdates(void *arg);
void Client_togglePause(Client *client);
void Client_handlePause(Client *client);
void Client_closeIfOpen(Client *client);

#endif
