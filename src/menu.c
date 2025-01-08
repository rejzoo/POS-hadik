
#include "menu.h"

const char *MenuOptionStrings[] = {
    "New Game",
    "Join Game",
    "Exit Game",
    "Continue Game"
};

void printMenu(WINDOW *menuWin, int highlight, MenuOption options[], int n_options) {
    int x = 2, y = 2;
    box(menuWin, 0, 0); // Draw a border around the menu
    for (int i = 0; i < n_options; ++i) {
        if (i == highlight) {
            wattron(menuWin, A_REVERSE); // Highlight selected item
            mvwprintw(menuWin, y, x, "%s", MenuOptionStrings[options[i]]);
            wattroff(menuWin, A_REVERSE);
        } else {
            mvwprintw(menuWin, y, x, "%s", MenuOptionStrings[options[i]]);
        }
        y++;
    }
    wrefresh(menuWin);
}

int displayMenu(MenuOption options[], int n_options) {
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
                choice = options[highlight];
                break;
        }
        if (choice != -1) break;
    }

    clear();
    refresh();
    endwin();

    return choice;
}

int mainMenu(char *mapSize, char *playerCount) {
  MenuOption options[] = {NEW_GAME, JOIN_GAME, EXIT_GAME}; 
  int choice = displayMenu(options, 3);

  if (choice == NEW_GAME) {
    newGameScreen(mapSize, playerCount); 
  }

  return choice;
}

int pauseMenu() {
  MenuOption options[] = {CONTINUE_GAME, NEW_GAME, JOIN_GAME, EXIT_GAME};
  return displayMenu(options, 4); 
}

int deathScreen() {
  MenuOption options[] = {JOIN_GAME, EXIT_GAME};
  return displayMenu(options, 2); 
}

void newGameScreen(char *mapSize, char *playerCount) {
    int height = 10, width = 40, startY = 4, startX = 4;
    WINDOW *inputWin = newwin(height, width, startY, startX);
    box(inputWin, 0, 0);
    mvwprintw(inputWin, 1, 2, "Enter Map Size:");
    mvwprintw(inputWin, 3, 2, "Enter Player Count:");

    echo();
    wrefresh(inputWin);

    mvwgetnstr(inputWin, 1, 18, mapSize, 4);
    mvwgetnstr(inputWin, 3, 20, playerCount, 4);

    noecho();

    delwin(inputWin);
}

