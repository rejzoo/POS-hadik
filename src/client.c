#include "client.h"

void startServer() {
    char mapSizeStr[5];
    fgets(mapSizeStr, sizeof(mapSizeStr), stdin);

    pid_t pid = fork();
    printf("PID: %d\n", pid);
    if (pid == 0) {
        execl("./server", "./server", mapSizeStr, NULL);
        perror("Failed to start server"); // Wont get there if the server starts
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        printf("SERVER: Process started with PID %d\n", pid);
    } else {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }
}

// Get insta input without printing
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

void drawGame(const DataFromServer *serverData) {
    clear();
    int map_size = serverData->map_size;

    // i j reversed
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

    refresh();
}

void parseData(const char *data, DataFromServer *serverData) {
    const char *ptr = data;

    sscanf(ptr, "%d", &serverData->map_size);
    ptr = strchr(ptr, ' ') + 1;

    sscanf(ptr, "%d", &serverData->n_snakes);
    ptr = strchr(ptr, ' ') + 1;

    sscanf(ptr, "%d", &serverData->n_bodies);
    ptr = strchr(ptr, ' ') + 1;

    for (int i = 0; i < serverData->n_snakes; i++) {
        sscanf(ptr, "%d %d", &serverData->snakes_heads[i].x, &serverData->snakes_heads[i].y);
        ptr = strchr(ptr, ' ') + 1;
        ptr = strchr(ptr, ' ') + 1; //Hack fix for bug
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

void *receiveUpdates(void *arg) {
    int client_fd = *(int *)arg;
    char buffer[1024];

    DataFromServer serverData;
    
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            parseData(buffer, &serverData);

            drawGame(&serverData);
            printf("DRAW\n");
        } else if (bytes_received == 0) {
            clear();
            refresh();
            int choice = deathScreen();
            handleChoice(choice);
            break;
        } else {
            perror("recv");
            break;
        }

    } 

    return NULL;
}

void joinGame() {
    sleep(2); // Wait a bit for server to start
    int client_fd;
    struct sockaddr_in serv_addr;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nCLIENT: Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nCLIENT: Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nCLIENT: Connection failed. \n");
        exit(EXIT_FAILURE);
    }

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    pthread_t update_thread;
    pthread_create(&update_thread, NULL, receiveUpdates, &client_fd);

    while (1) {
        char key = getKey();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd') {
            send(client_fd, &key, 1, 0);
        }

        if (key == 'q') break; // Quit on 'q'
    }

    close(client_fd);
    pthread_cancel(update_thread);
    pthread_join(update_thread, NULL);
    endwin();
}

void handleChoice(int choice) {
    switch(choice) {
        case 0: // New game
            startServer();
            joinGame();
            break;
        case 1: // Join game
            joinGame();
            break;
        case 2: // Exit
            exit(EXIT_SUCCESS);
    }
}

int main() {
    
    int choice = mainMenu();
    handleChoice(choice); 
    /*
    switch (option) {
        case 0: // New Game
            printf("Starting a new game...\n");
            startServer();
            joinGame();
            break;
        case 1: // Join Game
            printf("Joining an existing game...\n");
            joinGame();
            break;
        case 2: // Exit
            printf("Exiting...\n");
            exit(EXIT_SUCCESS);
            break;
    }*/

    return 0;
}

