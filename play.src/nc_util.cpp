#include <cctype>
#include <ncurses.h>
#include <string>

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

std::string getString(const std::string &prompt, unsigned maxlen, const std::string &initialText) {
    std::string text = initialText;
    int maxX = 0, maxY = 0;
    getmaxyx(stdscr, maxY, maxX);

    unsigned windowWidth = maxlen + 7;
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

    bkgdset(A_REVERSE | COLOR_PAIR(colorDialog));
    while (true) {
        move(top+3, left+3);
        for (unsigned i = 0; i < maxlen + 1; ++i) {
            if (i < text.size()) {
                addch(text[i]);
            } else if (i == text.size()) {
                addch('_');
            } else {
                addch(' ');
            }
        }
        refresh();
        int key = getch();
        if (isprint(key) && text.size() < maxlen) {
            text += static_cast<char>(key);
        }
        if (key == KEY_BACKSPACE && !text.empty()) {
            text.resize(text.size() - 1);
        }
        if (key == KEY_ENTER || key == ' ' || key == '\n' || key == '\r') {
            return text;
        }
    }
}