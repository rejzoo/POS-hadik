#include "server.h"
#include "snake.h"

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

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
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

int *acceptClientConnection(int server_fd, struct sockaddr_in *address) {
  int *new_socket = malloc(sizeof(int));
  socklen_t addrlen = sizeof(*address);

  if ((*new_socket = accept(server_fd, (struct sockaddr *)address, &addrlen)) <
      0) {
    perror("accept");
    free(new_socket);
    exit(EXIT_FAILURE);
  }
  return new_socket;
}

int generateCords(GameState *game) { return (rand() % game->map_size) + 1; }

void *handleClient(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  GameState *game = args->game;
  int client_socket = args->client_socket;

  free(args);
  char key;

  int flags = fcntl(client_socket, F_GETFL, 0);
  fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

  printf("Client connected: Socket: %d\n", client_socket);
  addClient(game, client_socket);

  Snake *snake = NULL;

  for (int i = 0; i < game->n_clients; i++) {
    if (snakeGetSocket(&game->snakes[i]) == client_socket) {
      snake = &game->snakes[i];
    }
  }

  while (snake != NULL && snakeAlive(snake)) {
    ssize_t bytes_read = read(client_socket, &key, 1);

    if (bytes_read > 0) {
      pthread_mutex_lock(&game->mutex);

          switch (key) {
          case 'w':
            snakeSetDirection(snake, 0, -1);
            break;
          case 's':
            snakeSetDirection(snake, 0, 1);
            break;
          case 'a':
            snakeSetDirection(snake, -1, 0);
            break;
          case 'd':
            snakeSetDirection(snake, 1, 0);
            break;
          }
      pthread_mutex_unlock(&game->mutex);
    } else if (bytes_read == 0) {
      printf("Client %d disconnected HANDLE CLIENT \n", client_socket);
      break;
    }
  }

  removeClient(game, client_socket);

  close(client_socket);
  printf("CLOSED AND REMOVED CLIENT: %d\n", client_socket);
  return NULL;
}

void addClient(GameState *game, int socket) {
  pthread_mutex_lock(&game->mutex);

    int index = -1;
    for (int i = 0; i < game->max_clients; i++) {
        if (!snakeAlive(&game->snakes[i])) {
            index = i;
            break;
        }
    }

    printf("ADD INDEX: %d\n", index);

    if (index == -1) {
        printf("Max clients reached. Rejecting client: %d\n", socket);
        close(socket); // Gracefully close the socket
        pthread_mutex_unlock(&game->mutex);
        return;
    }

    snakeInit(&game->snakes[index], socket, generateCords(game), generateCords(game));
    game->n_clients++;
    game->foods[index].x = generateCords(game);
    game->foods[index].y = generateCords(game);

    printf("SNAKE INDEX ADD: %d\n", snakeGetSocket(&game->snakes[index]));

    pthread_mutex_unlock(&game->mutex); 
}

void removeClient(GameState *game, int socket) {
  pthread_mutex_lock(&game->mutex);

    for (int i = 0; i < game->max_clients; i++) {
        if (snakeGetSocket(&game->snakes[i]) == socket) {
            printf("Removing client: %d at index %d\n", socket, i);

            snakeSetAlive(&game->snakes[i], 0);
            break;
        }
    }

    game->n_clients = 0;
    for (int i = 0; i < game->max_clients; i++) {
      if (snakeAlive(&game->snakes[i])) {
        game->n_clients++;
      }
    }

    pthread_mutex_unlock(&game->mutex);
}

void checkFoodCollision(GameState *game) {
  for (int i = 0; i < game->n_clients; i++) {
    if (snakeAlive(&game->snakes[i])) {
      for (int j = 0; j < game->n_clients; j++) {
        if (snakeGetX(&game->snakes[i]) == game->foods[j].x &&
            snakeGetY(&game->snakes[i]) == game->foods[j].y) {

          snakeGrow(&game->snakes[i]);

          game->foods[j].x = generateCords(game);
          game->foods[j].y = generateCords(game);
        }
      }
    }
  }
}

void checkSnakeCollision(GameState *game) {
  for (int i = 0; i < game->n_clients; i++) {
        if (snakeAlive(&game->snakes[i])) {
            Position head = snakeGetHead(&game->snakes[i]);

            for (int j = 0; j < game->n_clients; j++) {
                if (snakeAlive(&game->snakes[j])) {
                    int body_length = snakeGetBodyLength(&game->snakes[j]);
                    for (int k = 0; k < body_length; k++) {
                        Position body_part = snakeGetBodyPart(&game->snakes[j], k);

                        if (head.x == body_part.x && head.y == body_part.y) {
                            snakeSetAlive(&game->snakes[i], 0);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void broadcastGameState(GameState *game) {
  pthread_mutex_lock(&game->mutex);

  char buffer[1024];
  int offset = 0;

  offset +=
      snprintf(buffer + offset, sizeof(buffer) - offset, "%d ", game->map_size);

  int alive_snakes = 0;
  int total_bodies = 0;

  for (int i = 0; i < game->n_clients; i++) {
    if (snakeAlive(&game->snakes[i])) {
      alive_snakes++;
      total_bodies += snakeGetBodyLength(&game->snakes[i]);
    }
  }

  offset +=
      snprintf(buffer + offset, sizeof(buffer) - offset, "%d ", alive_snakes);

  offset +=
      snprintf(buffer + offset, sizeof(buffer) - offset, "%d ", total_bodies);

  for (int i = 0; i < game->n_clients; i++) {
    if (snakeAlive(&game->snakes[i])) {
      Position snake_head = snakeGetHead(&game->snakes[i]);
      offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d %d ",
                         snake_head.x,
                         snake_head.y);
    }
  }

  for (int i = 0; i < game->n_clients; i++) {
    for (int j = 0; j < snakeGetBodyLength(&game->snakes[i]); j++) {
      Position body_part = snakeGetBodyPart(&game->snakes[i], j);
      offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d %d ",
                         body_part.x, body_part.y);
    }
  }

  for (int i = 0; i < game->n_clients; i++) {
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d %d ",
                       game->foods[i].x, game->foods[i].y);
  }

  for (int i = 0; i < game->n_clients; i++) {
    if (snakeAlive(&game->snakes[i])) {
      send(snakeGetSocket(&game->snakes[i]), buffer, strlen(buffer), 0);
    }
  }

  pthread_mutex_unlock(&game->mutex);
}

void *gameLoop(void *arg) {
  GameState *game = (GameState *)arg;

  while (1) {
    pthread_mutex_lock(&game->mutex);

    for (int i = 0; i < game->n_clients; i++) {
      if (snakeAlive(&game->snakes[i])) {
        snakeMove(&game->snakes[i]);

        if (outOfBounds(&game->snakes[i], game)) {
          snakeSetAlive(&game->snakes[i], 0);
        }
      }
    }

    checkFoodCollision(game);
    checkSnakeCollision(game);
    pthread_mutex_unlock(&game->mutex);

    broadcastGameState(game);
    sleep(2);
  }
  return NULL;
}

int outOfBounds(Snake *snake, GameState *game) {
  if (snakeGetX(snake) < 1 || snakeGetX(snake) >= game->map_size + 1 ||
      snakeGetY(snake) < 1 || snakeGetY(snake) >= game->map_size + 1) {
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (argc < 3) {
    perror("Invalid number of arguments");
    exit(EXIT_FAILURE);
  }

  int map_size = atoi(argv[1]);
  int max_clients = atoi(argv[2]);

  if (map_size <= 0 || max_clients <= 0) {
    perror("Map size and max clients must be > 0");
    exit(EXIT_FAILURE);
  }

  GameState game = {
      .snakes = malloc(max_clients * sizeof(Snake)),
      .foods = malloc(max_clients * sizeof(Position)),
      .n_clients = 0,
      .max_clients = max_clients,
      .map_size = map_size,
      .mutex = PTHREAD_MUTEX_INITIALIZER
  };

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
    printf("Clients: %d\n", game.n_clients);
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

  printf("Server closed\n");
  free(game.snakes);
  free(game.foods);

  close(server_fd);

  return EXIT_SUCCESS;
}

