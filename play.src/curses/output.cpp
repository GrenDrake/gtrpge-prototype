#include <ncurses.h>
#include <sstream>

#include "play.h"


void drawOptions(Game &game) {
    int maxX = 0, maxY = 0;
    getmaxyx(stdscr, maxY, maxX);
    bkgdset(A_NORMAL | COLOR_PAIR(colorOptions));

    for (int i = 0; i < 5; ++i) {
        move(maxY-1-i, 0);
        clrtoeol();
    }

    int num = 1, counter = 0;
    for (auto &option : game.options) {
        std::stringstream ss;
        ss << (num++) << ") ";
        if (option.name == optionNameContinue) {
            ss << "Continue";
        } else if (option.name == optionNameCancel) {
            ss << "Cancel";
        } else if (option.name == optionDoNothing) {
            ss << "Do nothing";
        } else {
            ss << toUpperFirst(game.getNameOf(option.name));
        }
        int x, y = 0;
        if (counter < 5) {
            x = 0;
            y = maxY - 5 + counter;
        } else {
            x = maxX / 2;
            y = maxY - 5 + (counter - 5);
        }
        ++counter;
        mvprintw(y, x, ss.str().c_str());
    }
}

void drawOutput(Game &game) {
    int maxX = 0, maxY = 0;
    getmaxyx(stdscr, maxY, maxX);
    bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
    auto lines = explodeString(getOutput());

    const int maxLines = maxY-6;
    int lineCount = 0;

    while (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }

    for (int i = lines.size() - 1; i >= 0 && lineCount < maxLines; --i) {
        auto paragraph = wrapString(lines[i], maxX);
        for (int j = paragraph.size() - 1; j >= 0 && lineCount < maxLines; --j) {
            mvprintw(maxLines-lineCount, 0, paragraph[j].c_str());
            ++lineCount;
        }
        ++lineCount;
    }
}
