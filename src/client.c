#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include "menu.h"

#define PORT 8090

void startServer() {
    pid_t pid = fork();
    printf("PID: %d\n", pid);
    if (pid == 0) {
        execl("./server", "./server", NULL);
        perror("Failed to start server"); // Wont get there if the server starts
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

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read a single character
    ch = getchar();

    // Restore old terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

// Function to join the game by connecting to the server
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

    while (1) {
        char key = getKey();
        send(client_fd, &key, 1, 0);
        if (key == 'q') break; // Quit on 'q'
    }

    close(client_fd);
}

int main() {
    
    int option = mainMenu();
    
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
    }

    return 0;
}

