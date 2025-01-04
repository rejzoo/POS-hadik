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

int generateCords(GameState *game) {
     return (rand() % (game->map_size - 1)) + 2;
}

void *handleClient(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    GameState *game = args->game;
    int client_socket = args->client_socket;

    free(args);
    char key;
    
    printf("Client connected: Socket: %d\n", client_socket);
    addClient(game, client_socket);
   
    while (read(client_socket, &key, 1) > 0) {
        pthread_mutex_lock(&game->mutex);
        for (int i = 0; i < game->n_clients; i++) {
            if (game->snakes[i].socket == client_socket && game->snakes[i].alive) {
                switch (key) {
                    case 'w': game->snakes[i].dx = 0; game->snakes[i].dy = -1; break;
                    case 's': game->snakes[i].dx = 0; game->snakes[i].dy = 1; break;
                    case 'a': game->snakes[i].dx = -1; game->snakes[i].dy = 0; break;
                    case 'd': game->snakes[i].dx = 1; game->snakes[i].dy = 0; break;
                }
                
                break;
            }
        }
        
        pthread_mutex_unlock(&game->mutex);
    }

    removeClient(game, client_socket);
    close(client_socket);
    return NULL;
}

void initSnake(Snake *snake, int socket, int start_x, int start_y) {
    snake->socket = socket;
    snake->x = start_x;
    snake->y = start_y;
    snake->dx = 1;
    snake->dy = 0;
    snake->alive = 1;
}

void addClient(GameState *game, int socket) {
    pthread_mutex_lock(&game->mutex);
    
    int index = game->n_clients++;
    initSnake(&game->snakes[index], socket, generateCords(game), generateCords(game));
    game->food[index].x = generateCords(game);
    game->food[index].y = generateCords(game);
  
    pthread_mutex_unlock(&game->mutex);
}  

void removeClient(GameState *game, int socket) {
    pthread_mutex_lock(&game->mutex);

    for (int i = 0; i < game->n_clients; i++) {
      if (game->snakes[i].socket == socket) {
        game->snakes[i].alive = 0;
        printf("Client %d disconnected. \n", socket);
        break;
      }
    }

    pthread_mutex_unlock(&game->mutex);
}

void checkFoodCollision(GameState *game) {
  for (int i = 0; i < game->n_clients; i++) {
        if (game->snakes[i].alive) {
            for (int j = 0; j < game->n_clients; j++) {
                if (game->snakes[i].x == game->food[j].x && game->snakes[i].y == game->food[j].y) {
                    game->food[j].x = generateCords(game);
                    game->food[j].y = generateCords(game);
                }
            }
        }
    }
}

void broadcastGameState(GameState *game) {
    pthread_mutex_lock(&game->mutex);
    
    char buffer[1024];
    int offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d ", game->map_size);

    int alive_snakes = 0;
    for (int i = 0; i < game->n_clients; i++) {
        if (game->snakes[i].alive) {
            alive_snakes++;
        }
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d ", alive_snakes);

    for (int i = 0; i < game->n_clients; i++) {
        if (game->snakes[i].alive) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d %d ", game->snakes[i].x, game->snakes[i].y);
        }
    }

    for (int i = 0; i < game->n_clients; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d %d ", game->food[i].x, game->food[i].y);
    }
        
    for (int i = 0; i < game->n_clients; i++) {
        if (game->snakes[i].alive) {
            send(game->snakes[i].socket, buffer, strlen(buffer), 0);
        }
    }

    pthread_mutex_unlock(&game->mutex);
}

void *gameLoop(void *arg) {
    GameState *game = (GameState *)arg;

    while (1) {
        pthread_mutex_lock(&game->mutex);

        for (int i = 0; i < game->n_clients; i++) {
            if (game->snakes[i].alive) {
                game->snakes[i].x += game->snakes[i].dx;
                game->snakes[i].y += game->snakes[i].dy;

                if (game->snakes[i].x < 1 || game->snakes[i].x >= game->map_size + 1 ||
                    game->snakes[i].y < 1 || game->snakes[i].y >= game->map_size + 1) {
                    game->snakes[i].alive = 0;
                }
            }
        }

        checkFoodCollision(game);
        pthread_mutex_unlock(&game->mutex);

        broadcastGameState(game);

        sleep(2);
        //usleep(300000);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int map_size = atoi(argv[1]);

    GameState game = { .n_clients = 0, .map_size = map_size, .mutex = PTHREAD_MUTEX_INITIALIZER };
    
    int server_fd;
    struct sockaddr_in address;

    server_fd = createServerSocket();
    configureServerAddress(&address);
    bindAndListen(server_fd, &address);
    printf("Server started\n");
    
    pthread_t game_td;
    pthread_create(&game_td, NULL, gameLoop, &game);
    
    while (1) {
      int *new_socket = acceptClientConnection(server_fd, &address);

      ThreadArgs *args = malloc(sizeof(ThreadArgs));
      args->game = &game;
      args->client_socket = *new_socket;
    
      pthread_t client;
      
      if (pthread_create(&client, NULL, handleClient, args) != 0) {
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

