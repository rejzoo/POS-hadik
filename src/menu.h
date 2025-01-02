
#ifndef MENU_H
#define MENU_H

#include <ncurses.h>

#define MAIN_MENU_OPTIONS 3
#define PAUSED_MENU_OPTIONS 4

void printMenu(WINDOW *menuWin, int highlight, char *options[], int n_options);
int displayMenu(char *options[], int n_options);
int mainMenu();
int pauseMenu();

#endif
