#include "client.h"

void Client_init(Client *client) {
    client->client_fd = -1;
    client->client_alive = 1;
    client->server_data.snakes_heads = NULL;
    client->server_data.snakes_bodies = NULL;
    client->server_data.foods = NULL;
}

void Client_startServer() {
    char map_size[5];
    char max_clients[3];

    printf("Map size: ");
    fgets(map_size, sizeof(map_size), stdin);
    printf("Max clients: ");
    fgets(max_clients, sizeof(max_clients), stdin);

    pid_t pid = fork();
    printf("PID: %d\n", pid);
    if (pid == 0) {
        execl("./SnakeGameServer", "./SnakeGameServer", map_size, max_clients, NULL);
        perror("Failed to start server");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        printf("SERVER: Process started with PID %d\n", pid);
    } else {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }
}

char getKey() {
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

void Client_drawGame(const DataFromServer *serverData) {
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

    printw("Number of players: %d", serverData->n_snakes);

    refresh();
}

void Client_parseData(const char *data, DataFromServer *serverData) {
    const char *ptr = data;

    sscanf(ptr, "%d", &serverData->map_size);
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

    while (1) {
        ssize_t bytes_received = recv(client->client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            Client_freeDataFromServer(&client->server_data);
            Client_parseData(buffer, &client->server_data);
            Client_drawGame(&client->server_data);
        } else if (bytes_received == 0) {
            client->client_alive = 0;
            break;
        } else {
            perror("recv");
            break;
        }
    }

    return NULL;
}

void Client_joinGame(Client *client) {
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

    pthread_create(&client->update_thread, NULL, Client_receiveUpdates, client);

    while (client->client_alive) {
        char key = getKey();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd') {
            send(client->client_fd, &key, 1, 0);
        }
        if (key == 'q') break;
    }

    close(client->client_fd);
    pthread_cancel(client->update_thread);
    pthread_join(client->update_thread, NULL);
    endwin();
}

void Client_handleChoice(Client *client, int choice) {
    switch (choice) {
        case 0:
            Client_startServer();
            Client_joinGame(client);
            break;
        case 1:
            Client_joinGame(client);
            break;
        case 2:
            exit(EXIT_SUCCESS);
    }
}

int main() {
    Client client;
    Client_init(&client);

    int choice = mainMenu();
    Client_handleChoice(&client, choice);

    choice = deathScreen();
    Client_handleChoice(&client, choice);

    return 0;
}

