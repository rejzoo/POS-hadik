
#include "menu.h"

const char *MenuOptionStrings[] = {
    "New Game",
    "Join Game",
    "Exit Game",
    "Continue Game"
};

void printMenu(WINDOW *menuWin, int highlight, MenuOption options[], int n_options, char *text) {
    int x = 2, y = 2;
    box(menuWin, 0, 0); // Draw border around the menu
    mvwprintw(menuWin, y, x, "%s", text);
    y+= 2;
    
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

int displayMenu(MenuOption options[], int n_options, char *text) {
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
        printMenu(menuWin, highlight, options, n_options, text);
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
  char *text = "Main menu";
  int choice = displayMenu(options, 3, text);

  if (choice == NEW_GAME) {
    newGameScreen(mapSize, playerCount); 
  }

  return choice;
}

int pauseMenu(char *mapSize, char *playerCount) {
  MenuOption options[] = {CONTINUE_GAME, NEW_GAME, JOIN_GAME, EXIT_GAME};
  char *text = "Pause menu";
  int choice = displayMenu(options, 4, text); 

  if (choice == NEW_GAME) {
    newGameScreen(mapSize, playerCount);
  }

  return choice;
}

int deathScreen(int score) {
  MenuOption options[] = {JOIN_GAME, EXIT_GAME};
  char text[100];
  snprintf(text, sizeof(text), "You died! Score: %d", score);
  return displayMenu(options, 2, text); 
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

