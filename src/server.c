#include "server.h"

int createServerSocket() {
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void configureServerAddress(struct sockaddr_in *address) {
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
}

void bindAndListen(int server_fd, struct sockaddr_in *address) {
    int opt = 1;
    socklen_t addrlen = sizeof(*address);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int* acceptClientConnection(int server_fd, struct sockaddr_in *address) {
    int *new_socket = malloc(sizeof(int));
    socklen_t addrlen = sizeof(*address);

    if ((*new_socket = accept(server_fd, (struct sockaddr *)address, &addrlen)) < 0) {
        perror("accept");
        free(new_socket);
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

void *handleClient(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char key = 0;
    
    printf("Client connected: Socket: %d\n", client_socket);

    while (1) {
      ssize_t bytes_read = read(client_socket, &key, 1);

      if (bytes_read > 0) {
          printf("Client %d sent: %c\n", client_socket, key);
      } else if (bytes_read == 0) {
          printf("Client %d disconnected.\n", client_socket);
          break;
      } else {
          perror("Error read");
          break;
      }
    }
    
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("Server init\n");
    int server_fd;
    struct sockaddr_in address;

    server_fd = createServerSocket();
    configureServerAddress(&address);
    bindAndListen(server_fd, &address);
    printf("Server started\n");

    while (1) {
      int *new_socket = acceptClientConnection(server_fd, &address);
    
      pthread_t client;
      
      if (pthread_create(&client, NULL, handleClient, new_socket) != 0) {
          perror("Failed pthread_create");
          close(*new_socket);
          free(new_socket);
      } else {
          pthread_detach(client);
      }
    }

    close(server_fd);

    return EXIT_SUCCESS;
}

