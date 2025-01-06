
#include "menu.h"

void printMenu(WINDOW *menuWin, int highlight, char *options[], int n_options) {
    int x = 2, y = 2;
    box(menuWin, 0, 0); // Draw a border around the menu
    for (int i = 0; i < n_options; ++i) {
        if (i == highlight) {
            wattron(menuWin, A_REVERSE); // Highlight selected item
            mvwprintw(menuWin, y, x, "%s", options[i]);
            wattroff(menuWin, A_REVERSE);
        } else {
            mvwprintw(menuWin, y, x, "%s", options[i]);
        }
        y++;
    }
    wrefresh(menuWin);
}

int displayMenu(char *options[], int n_options) {
    int highlight = 0;
    int choice = -1;

    // Initialize ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    int height = 10, width = 30, startY = 4, startX = 4;
    WINDOW *menuWin = newwin(height, width, startY, startX);
    keypad(menuWin, TRUE);

    while (1) {
        printMenu(menuWin, highlight, options, n_options);
        int c = wgetch(menuWin);
        printf("%c\n", c);
        switch (c) {
            case KEY_UP:
                if (highlight > 0) highlight--;
                break;
            case KEY_DOWN:
                if (highlight < n_options - 1) highlight++;
                break;
            case '\n': // Enter
                choice = highlight;
                break;
        }
        if (choice != -1) break;
    }

    clear();
    refresh();
    endwin();

    return choice;
}

int mainMenu() {
  char *options[MAIN_MENU_OPTIONS] = {
    "New Game",
    "Join Game",
    "Exit"
  };

  return displayMenu(options, MAIN_MENU_OPTIONS);
}

int pauseMenu() {
  char *options[PAUSED_MENU_OPTIONS] = {
    "Continue Game",
    "New Game",
    "Join Game",
    "Exit"
  };

  return displayMenu(options, PAUSED_MENU_OPTIONS);
}

int deathScreen() {
  char *options[DEATHSCREEN_MENU_OPTIONS] = {
    "Join Game",
    "Exit Game"
  };

  return displayMenu(options, DEATHSCREEN_MENU_OPTIONS);
}

