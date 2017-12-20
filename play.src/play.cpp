#include <iostream>
#include <string>

#include "play.h"

extern "C" {
#include "glk.h"
}



static void update(Game &game, GameIO &io) {
    io.setWindow(GameIO::Options);
    io.clear();
    int num = 1;
    for (auto &option : game.options) {
        io.say(num++);
        io.say(") ");
        if (option.name == 1) {
            io.say("Continue");
        } else {
            io.say(game.getString(option.name));
        }
        io.say("\n");
    }

    io.setWindow(GameIO::Status);
    io.clear();
    if (game.locationName) {
        io.say(game.getString(game.locationName));
}

    io.setWindow(GameIO::Main);
}

void glk_main() {
    GameIO io;
    Game game(io);
    io.setup();

    try {
        game.loadDataFromFile("game.bin");
        game.startGame();

        while (true) {
            update(game, io);

            int key = io.getKey();
            if (key >= '1' && key <= '9') {
                game.doOption(key - '1');
            } else if (key == 'Y' && game.actionAllowed()) {
                io.setWindow(GameIO::Sidebar);
                io.say("Hello there! Test text.\n");
                io.setWindow(GameIO::Main);
            } else if (key == 'Q') {
                io.style(GameIO::Emphasis);
                io.say("\nGoodbye!\n");
                return;
            }
        }

    } catch (PlayError &e) {
        std::cerr << "FATAL: " << e.what() << "\n";
    }

    return;
}