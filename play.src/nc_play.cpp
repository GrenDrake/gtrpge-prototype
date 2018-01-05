#include <cctype>
#include <iostream>
#include <ncurses.h>
#include <panel.h>
#include <string>
#include <sstream>
#include <vector>

#include "nc_play.h"

const int maxOutputBuffer = 4096;
char gamefile[64] = "game.bin";

WINDOW *statusWindow = nullptr;
PANEL *statusPanel = nullptr;
WINDOW *mainWindow = nullptr;
PANEL *mainPanel = nullptr;
WINDOW *optionsWindow = nullptr;
PANEL *optionsPanel = nullptr;
WINDOW *subWindow = nullptr;
PANEL *subPanel = nullptr;

std::string outputBuffer;

void addToOutput(const std::string &text) {
    outputBuffer += text;
    if (outputBuffer.size() > maxOutputBuffer) {
        outputBuffer.erase(0, outputBuffer.size() - maxOutputBuffer);
    }
}


static void drawStatus(Game &game) {
    if (!game.gameStarted) return;

    wclear(statusWindow);
    wprintw(statusWindow, "  (I)nventory   (C)haracter Sheets    (Q)uit\n");
    if (game.locationName) {
        mvwprintw(statusWindow, 1, 2, game.getString(game.locationName));
    }
    mvwprintw(statusWindow, 1, COLS-20, toUpperFirst(game.getTimeString()).c_str());
}

static void drawOptions(Game &game) {
    wclear(optionsWindow);
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
            y = counter;
        } else {
            x = COLS / 2;
            y = counter - 5;
        }
        ++counter;
        mvwprintw(optionsWindow, y, x, ss.str().c_str());
    }
}

static void drawOutput(Game &game) {
    wclear(mainWindow);

    auto lines = explodeString(outputBuffer);

    const int maxLines = LINES-7;
    int lineCount = 0;

    for (int i = lines.size() - 1; i >= 0 && lineCount < maxLines; --i) {
        auto paragraph = wrapString(lines[i], COLS);
        for (int j = paragraph.size() - 1; j >= 0 && lineCount < maxLines; --j) {
            mvwprintw(mainWindow, maxLines-1-lineCount, 0, paragraph[j].c_str());
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
        drawStatus(game);
        drawOutput(game);
        drawOptions(game);
        update_panels();
        doupdate();

        int realKey = wgetch(optionsWindow);
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
    start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);  // for options window
	init_pair(2, COLOR_WHITE, COLOR_BLACK); // for main window
	init_pair(3, COLOR_BLACK, COLOR_WHITE); // for sub window
	init_pair(4, COLOR_BLACK, COLOR_WHITE); // for status window

    statusWindow = newwin(2, COLS, 0, 0);
    statusPanel = new_panel(statusWindow);
    wbkgd(statusWindow, COLOR_PAIR(4));

    mainWindow = newwin(LINES - 7, COLS, 2, 0);
    mainPanel = new_panel(mainWindow);
    wbkgd(mainWindow, COLOR_PAIR(2));

    optionsWindow = newwin(5, COLS, LINES - 5, 0);
    optionsPanel = new_panel(optionsWindow);
    wbkgd(optionsWindow, COLOR_PAIR(1));
    keypad(optionsWindow, true);

    subWindow = newwin(LINES - 6, COLS - 6, 3, 3);
    subPanel = new_panel(subWindow);
    wbkgd(subWindow, COLOR_PAIR(3));
    hide_panel(subPanel);
    keypad(subWindow, true);


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