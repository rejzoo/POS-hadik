#include "snake.h"

void snakeInit(Snake *snake, int socket, int start_x, int start_y) {
    snake->socket = socket;
    snake->x = start_x;
    snake->y = start_y;
    snake->dx = 1;
    snake->dy = 0;
    snake->alive = 1;
    snake->n_body = 0;
}

void destroySnake(Snake *snake) {
    if (snake) {
        free(snake);
    }
}

void snakeMove(Snake *snake) {
    if (!snake->alive) return;

    if (snake->n_body > 0) {
        for (int i = snake->n_body - 1; i > 0; i--) {
            snake->body[i] = snake->body[i - 1];
        }
        snake->body[0] = (Position){snake->x, snake->y};
    }

    snake->x += snake->dx;
    snake->y += snake->dy;
}

void snakeGrow(Snake *snake) {
    if (snake->n_body < MAX_LENGTH_SNAKE) {
        snake->body[snake->n_body++] = (Position){snake->x - snake->dx, snake->y - snake->dy};
    }
}

void snakeSetDirection(Snake *snake, int dx, int dy) {
    snake->dx = dx;
    snake->dy = dy;
}

void snakeSetAlive(Snake *snake, int status) {
    snake->alive = status;
}

int snakeAlive(const Snake *snake) {
    return snake->alive;
}

Position snakeGetHead(const Snake *snake) {
    return (Position){snake->x, snake->y};
}

int snakeGetX(Snake *snake) {
    return snake->x;
}

int snakeGetY(Snake *snake) {
    return snake->y;
}

int snakeGetBodyLength(Snake *snake) {
    return snake->n_body;
}

Position snakeGetBodyPart(Snake *snake, int index) {
    return snake->body[index];
}

int snakeGetSocket(Snake *snake) {
    return snake->socket;
}