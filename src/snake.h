
#ifndef SNAKE_H
#define SNAKE_H

#include <stdlib.h>
#include "utils.h"

typedef struct {
  Position body[MAX_LENGTH_SNAKE];
  int n_body;
  int socket;
  int x, y;
  int dx, dy;
  int alive;
  int score;
  int pause;
} Snake;

void snakeInit(Snake *snake, int socket, int start_x, int start_y);
void destroySnake(Snake *snake);
void snakeMove(Snake *snake);
void snakeGrow(Snake *snake);

void snakeSetDirection(Snake *snake, int dx, int dy);
void snakeSetAlive(Snake *snake, int status);
void snakeSetPause(Snake *snake, int status);

int snakeAlive(const Snake *snake);
Position snakeGetHead(const Snake *snake);
int snakeGetX(Snake *snake);
int snakeGetY(Snake *snake);
int snakeGetPause(Snake *snake);
int snakeGetBodyLength(Snake *snake);
Position snakeGetBodyPart(Snake *snake, int index);
int snakeGetSocket(Snake *snake);
void snakeTogglePause(Snake *snake);

#endif
