
#ifndef MENU_H
#define MENU_H

#include <ncurses.h>
#include <stdlib.h>

typedef enum {
    NEW_GAME = 0,
    JOIN_GAME = 1,
    EXIT_GAME = 2,
    CONTINUE_GAME = 3
} MenuOption;

void printMenu(WINDOW *menuWin, int highlight, MenuOption options[], int n_options, char *text);
int displayMenu(MenuOption options[], int n_options, char *text);
int mainMenu(char *mapSize, char *playerCount);
int pauseMenu(char *mapSize, char *playerCount);
int deathScreen(int score);
void newGameScreen(char *mapSize, char *playerCount);

#endif
