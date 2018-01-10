#include <cctype>
#include <ncurses.h>

#include "nc_play.h"

bool getYesNo(const std::string &prompt, bool defaultAnswer) {
    int maxX = 0, maxY = 0;
    getmaxyx(stdscr, maxY, maxX);

    unsigned windowWidth = prompt.size() + 6;
    unsigned windowHeight = 5;
    unsigned top  = (maxY - windowHeight) / 2;
    unsigned left = (maxX - windowWidth)  / 2;

    bkgdset(A_NORMAL | COLOR_PAIR(colorDialog));
    for (unsigned y = 0; y < windowHeight; ++y) {
        for (unsigned x = 0; x < windowWidth; ++x) {
            mvaddch(y+top, x+left, ' ');
        }
        mvaddch(y+top, left, ACS_VLINE);
        mvaddch(y+top, left+windowWidth-1, ACS_VLINE);
    }
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, left+windowWidth-1, ACS_URCORNER);
    mvaddch(top+windowHeight-1, left, ACS_LLCORNER);
    mvaddch(top+windowHeight-1, left+windowWidth-1, ACS_LRCORNER);
    mvprintw(top+1, left+3, prompt.c_str());
    if (defaultAnswer) {
        mvprintw(top+3, (maxX-10)/2, "YES or no?");
    } else {
        mvprintw(top+3, (maxX-10)/2, "yes or NO?");
    }

    refresh();
    while (true) {
        int key = toupper(getch());
        if (key == 'Y') return true;
        if (key == 'N') return false;
        if (key == KEY_ENTER || key == ' ' || key == '\n' || key == '\r') {
            return defaultAnswer;
        }
    }
}