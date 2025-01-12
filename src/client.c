#include "client.h"

void Client_init(Client *client) {
    client->client_fd = -1;
    client->client_alive = 1;
    client->client_score = 0;
    client->client_paused = 0;
    client->end_game = 0;
    client->server_data.snakes_heads = NULL;
    client->server_data.snakes_bodies = NULL;
    client->server_data.foods = NULL;
}

void Client_startServer(char *map_size, char *max_clients, char *game_mode, char *game_time) {
    pid_t pid = fork();
    printf("PID: %d\n", pid);
    if (pid == 0) {
        execl("./SnakeGameServer", "./SnakeGameServer", map_size, max_clients, game_mode, game_time, NULL);
        perror("Failed to start server");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        printf("SERVER: Process started with PID %d\n", pid);
    } else {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }
}

void Client_drawGame(const DataFromServer *serverData, Client *client) {
    if (client->client_paused) return;
    clear();
    int map_size = serverData->map_size;

    char grid[map_size + 2][map_size + 2];
    for (int i = 0; i < map_size + 2; i++) {
        for (int j = 0; j < map_size + 2; j++) {
            if ((i == 0 || i == map_size + 1) || (j == 0 || j == map_size + 1)) {
                grid[j][i] = '#';
            } else {
                grid[j][i] = ' ';
            }
        }
    }

    for (int i = 0; i < serverData->n_snakes; i++) {
        grid[serverData->snakes_heads[i].x][serverData->snakes_heads[i].y] = 'O';
    }

    for (int i = 0; i < serverData->n_bodies; i++) {
        grid[serverData->snakes_bodies[i].x][serverData->snakes_bodies[i].y] = 'o';
    }

    for (int i = 0; i < serverData->n_snakes; i++) {
        grid[serverData->foods[i].x][serverData->foods[i].y] = '+';
    }

    for (int i = 0; i < map_size + 2; i++) {
        for (int j = 0; j < map_size + 2; j++) {
            printw("%c", grid[j][i]);
        }
        printw("\n");
    }

    printw("Number of players: %d\n", serverData->n_snakes);
    printw("PAUSED: %d\n", client->client_paused);
    printw("TIME: %d\n", serverData->time);
    printw("Score: %d", client->client_score);

    refresh();
    usleep(50000);
}

void Client_parseData(const char *data1, const char *data2, DataFromServer *serverData, Client *client) {
    const char *ptr = data1;
    const char *ptr2 = data2;

    sscanf(ptr, "%d", &serverData->map_size);
    ptr = strchr(ptr, ' ') + 1;

    sscanf(ptr, "%d", &serverData->time);
    ptr = strchr(ptr, ' ') + 1;

    sscanf(ptr, "%d", &serverData->n_snakes);
    ptr = strchr(ptr, ' ') + 1;

    sscanf(ptr, "%d", &serverData->n_bodies);
    ptr = strchr(ptr, ' ') + 1;

    serverData->snakes_heads = malloc(serverData->n_snakes * sizeof(Position));
    serverData->snakes_bodies = malloc(serverData->n_bodies * sizeof(Position));
    serverData->foods = malloc(serverData->n_snakes * sizeof(Position));

    for (int i = 0; i < serverData->n_snakes; i++) {
        sscanf(ptr, "%d %d", &serverData->snakes_heads[i].x, &serverData->snakes_heads[i].y);
        ptr = strchr(ptr, ' ') + 1;
        ptr = strchr(ptr, ' ') + 1;
    }

    for (int i = 0; i < serverData->n_bodies; i++) {
        sscanf(ptr, "%d %d", &serverData->snakes_bodies[i].x, &serverData->snakes_bodies[i].y);
        ptr = strchr(ptr, ' ') + 1;
        ptr = strchr(ptr, ' ') + 1;
    }

    for (int i = 0; i < serverData->n_snakes; i++) {
        sscanf(ptr, "%d %d", &serverData->foods[i].x, &serverData->foods[i].y);
        ptr = strchr(ptr, ' ') + 1;
        ptr = strchr(ptr, ' ') + 1;
    }

    //sscanf(ptr2, "%d", &client->client_score);
}

void Client_freeDataFromServer(DataFromServer *serverData) {
    if (serverData->snakes_heads) {
        free(serverData->snakes_heads);
        serverData->snakes_heads = NULL;
    }
    if (serverData->snakes_bodies) {
        free(serverData->snakes_bodies);
        serverData->snakes_bodies = NULL;
    }
    if (serverData->foods) {
        free(serverData->foods);
        serverData->foods = NULL;
    }
}

void *Client_receiveUpdates(void *arg) {
    Client *client = (Client *)arg;
    char buffer[1024];
    char clientBuffer[256];

    while (client->client_alive) {
        ssize_t bytes_received1 = recv(client->client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received1 > 0) {
            buffer[bytes_received1] = '\0';
        } else if (bytes_received1 == 0) {
            client->client_alive = 0;
            break;
        } else {
            perror("recv");
            break;
        }

        //ssize_t bytes_received2 = recv(client->client_fd, clientBuffer, sizeof(clientBuffer) - 1, 0);
        //if (bytes_received2 > 0) {
        //    clientBuffer[bytes_received2] = '\0';
        //} else if (bytes_received2 == 0) {
        //   client->client_alive = 0;
        //    break;
        //} else {
        //   perror("recv");
        //    break;
        //}

      Client_freeDataFromServer(&client->server_data);
      Client_parseData(buffer, clientBuffer, &client->server_data, client);
      Client_drawGame(&client->server_data, client);
    }

    Client_freeDataFromServer(&client->server_data);
    return NULL;
}

void *Client_input(void *arg) {
      Client *client = (Client *)arg;

      while (client->client_alive) {
        if (client->client_paused) {
            usleep(100000);
            continue;
        }

        char key = getch();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd' || key == 'p') {
            if (key == 'p') Client_togglePause(client);
            send(client->client_fd, &key, 1, 0);
        }
        if (key == 'q') break;

        usleep(100000);
    }

    return NULL;
}

void Client_joinGame(Client *client) {
    printf("Joining server\n");
    sleep(2);
    struct sockaddr_in serv_addr;

    if ((client->client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nCLIENT: Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nCLIENT: Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(client->client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nCLIENT: Connection failed. \n");
        exit(EXIT_FAILURE);
    }

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    pthread_create(&client->update_thread, NULL, Client_receiveUpdates, client);
    pthread_create(&client->input_thread, NULL, Client_input, client);

    pthread_detach(client->update_thread);
    pthread_detach(client->input_thread);
}

void Client_handleChoice(Client *client, int choice, char *map_size, char *max_clients, char *game_mode, char *game_time) {
    switch (choice) {
        case 0:
            Client_closeIfOpen(client);
            Client_startServer(map_size, max_clients, game_mode, game_time);
            Client_joinGame(client);
            break;
        case 1:
            Client_closeIfOpen(client);
            close(client->client_fd);
            Client_joinGame(client);
            break;
        case 2:
            Client_closeIfOpen(client);
            client->end_game = 1;
        case 3: {
            char key = 'p';
            send(client->client_fd, &key, 1, 0);
            Client_togglePause(client);
            break;
          }
    }
}

void Client_togglePause(Client *client) {
    if (client->client_paused == 0) {
      client->client_paused = 1;
    } else {
      client->client_paused = 0;
    }
}

void Client_closeIfOpen(Client *client) {
    if (client->client_fd != -1) {
      close(client->client_fd);
      pthread_cancel(client->input_thread);
      pthread_cancel(client->update_thread);
    }

    Client_init(client);
}

void Client_handlePause(Client *client) {
    char map_size[5];
    char max_clients[5];
    int choice = pauseMenu(map_size, max_clients);
    Client_handleChoice(client, choice, map_size, max_clients, NULL, NULL);
}

int main() {
    Client client;
    char map_size[5];
    char max_clients[5];
    char game_mode[5];
    char game_time[9999];

    Client_init(&client);
    int choice = mainMenu(map_size, max_clients, game_mode, game_time);
    Client_handleChoice(&client, choice, map_size, max_clients, game_mode, game_time);

    while (!client.end_game) {
      if (!client.client_alive) {
        clear();
        refresh();
        choice = deathScreen(client.client_score);
        Client_handleChoice(&client, choice, NULL, NULL, NULL, NULL);
      } else if (client.client_paused) {
        choice = pauseMenu(map_size, max_clients);
        Client_handleChoice(&client, choice, map_size, max_clients, game_mode, game_time);
      }
      usleep(100000);
    }

    if (client.client_fd != -1) {
      close(client.client_fd);
    }

    endwin();
    return 0;
}
