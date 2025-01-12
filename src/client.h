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
#include <fcntl.h>
#include "menu.h"

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
    int client_score;
    int client_paused;
    int end_game;
    pthread_t update_thread;
    pthread_t input_thread;
    DataFromServer server_data;
} Client;

void Client_init(Client *client);
void Client_startServer(char *map_size, char *max_clients);
void Client_joinGame(Client *client);
void Client_handleChoice(Client *client, int choice, char *map_size, char *max_clients);
void Client_drawGame(const DataFromServer *serverData, Client *client);
void Client_parseData(const char *data1, const char *data2, DataFromServer *serverData, Client *client);
void Client_freeDataFromServer(DataFromServer *serverData);
void *Client_receiveUpdates(void *arg);
void Client_togglePause(Client *client);
void Client_handlePause(Client *client);
void Client_closeIfOpen(Client *client);

#endif

