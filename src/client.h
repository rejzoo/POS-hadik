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

// Constants
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

typedef struct {
    int client_fd;
    int client_alive;
    pthread_t update_thread;
    DataFromServer server_data;
} Client;

// Client methods
void Client_init(Client *client);
void Client_startServer();
void Client_joinGame(Client *client);
void Client_handleChoice(Client *client, int choice);
void Client_drawGame(const DataFromServer *serverData);
void Client_parseData(const char *data, DataFromServer *serverData);
void Client_freeDataFromServer(DataFromServer *serverData);
void *Client_receiveUpdates(void *arg);

#endif
