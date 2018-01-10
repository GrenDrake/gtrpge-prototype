#include <cctype>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <sstream>
#include <vector>

#include "nc_play.h"

const int maxOutputBuffer = 4096;
char gamefile[64] = "game.bin";

std::string outputBuffer;

void addToOutput(const std::string &text) {
    outputBuffer += text;
    if (outputBuffer.size() > maxOutputBuffer) {
        outputBuffer.erase(0, outputBuffer.size() - maxOutputBuffer);
    }
}


void drawStatus(Game &game) {
    int maxX = getmaxx(stdscr);
    bkgdset(A_NORMAL | COLOR_PAIR(colorStatus));
    move(0, 0); clrtoeol();

    if (!game.gameStarted) return;

    if (game.locationName) {
        mvprintw(0, 3, game.getString(game.locationName));
    }
    mvprintw(0, maxX-20, toUpperFirst(game.getTimeString()).c_str());
}

static void drawOptions(Game &game) {
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
        if (option.name == 1) {
            ss << "Continue";
        } else {
            ss << game.getString(option.name);
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

static void drawOutput(Game &game) {
    int maxX = 0, maxY = 0;
    getmaxyx(stdscr, maxY, maxX);
    bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
    auto lines = explodeString(outputBuffer);

    const int maxLines = maxY-6;
    int lineCount = 0;

    for (int i = lines.size() - 1; i >= 0 && lineCount < maxLines; --i) {
        auto paragraph = wrapString(lines[i], maxX);
        for (int j = paragraph.size() - 1; j >= 0 && lineCount < maxLines; --j) {
            mvprintw(maxLines-lineCount, 0, paragraph[j].c_str());
            ++lineCount;
        }
        ++lineCount;
    }
}

void gameloop() {
    Game game;

    game.loadDataFromFile(gamefile);
    game.startGame();
    addToOutput(game.getOutput());

    while (true) {
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        clear();
        drawStatus(game);
        drawOutput(game);
        drawOptions(game);
        refresh();

        int realKey = getch();
        int key = toupper(realKey);
        if (key >= '1' && key <= '9') {
            game.doOption(key - '1');
            addToOutput(game.getOutput());
    //     } else if (key == 'L') {
    //         if (io.hasTranscript()) {
    //             io.stopTranscript();
    //         } else {
    //             io.startTranscript();
    //         }
        } else if (key == ' ') {
            if (game.options.size() == 1) {
                game.doOption(0);
                addToOutput(game.getOutput());
            }
        } else if (key == 'I' && game.isRunning && game.actionAllowed()) {
            doInventory(game);
        } else if (key == 'C' && game.isRunning && game.actionAllowed()) {
            doCharacter(game);
        } else if (realKey == 'Q') {
            return;
        }
    }
}


int main(int argc, char *argv[]) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    start_color();
    curs_set(0);
	init_pair(colorOptions, COLOR_WHITE, COLOR_BLUE);  // for options window
	init_pair(colorMain,    COLOR_WHITE, COLOR_BLACK); // for main window
	init_pair(colorDialog,  COLOR_BLACK, COLOR_WHITE); // for sub window
	init_pair(colorStatus,  COLOR_BLACK, COLOR_WHITE); // for status window


    try {
        gameloop();
    } catch (PlayError &e) {
        endwin();
        std::cerr << "Fatal error occured: ";
        std::cerr << e.what() << "\n";
        return 1;
    }
    endwin();
    return 0;
}